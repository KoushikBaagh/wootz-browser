/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */
package org.chromium.chrome.browser.crypto_wallet.util;

import org.chromium.brave_wallet.mojom.SolanaProviderError; // koushik change
import org.chromium.brave_wallet.mojom.TransactionInfo; // koushik change
import org.chromium.brave_wallet.mojom.TransactionType; // koushik change
import org.chromium.chrome.browser.crypto_wallet.activities.BraveWalletBaseActivity; // koushik change

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;

public class SolanaTransactionsGasHelper {
    private final WeakReference<WootzWalletBaseActivity> mActivity;
    private TransactionInfo[] mTransactionInfos;
    private HashMap<String, Long> mPerTxFee;

    public SolanaTransactionsGasHelper(
            WootzWalletBaseActivity activity, TransactionInfo[] transactionInfos) {
        mActivity = new WeakReference<WootzWalletBaseActivity>(activity);
        mTransactionInfos = transactionInfos;
        mPerTxFee = new HashMap<String, Long>();
    }

    public HashMap<String, Long> getPerTxFee() {
        return mPerTxFee;
    }

    public void maybeGetSolanaGasEstimations(Runnable runWhenDone) {
        // Filter out everything that's not related to Solana
        ArrayList<TransactionInfo> solanaTransactions = new ArrayList<TransactionInfo>();
        for (TransactionInfo txInfo : mTransactionInfos) {
            if (txInfo.txType == TransactionType.SOLANA_SYSTEM_TRANSFER
                    || txInfo.txType == TransactionType.SOLANA_SPL_TOKEN_TRANSFER
                    || txInfo.txType
                            == TransactionType
                                       .SOLANA_SPL_TOKEN_TRANSFER_WITH_ASSOCIATED_TOKEN_ACCOUNT_CREATION
                    || txInfo.txType == TransactionType.SOLANA_DAPP_SIGN_AND_SEND_TRANSACTION
                    || txInfo.txType == TransactionType.SOLANA_DAPP_SIGN_TRANSACTION
                    || txInfo.txType == TransactionType.SOLANA_SWAP) {
                solanaTransactions.add(txInfo);
            }
        }
        AsyncUtils.MultiResponseHandler estimatesMultiResponse =
                new AsyncUtils.MultiResponseHandler(solanaTransactions.size());

        ArrayList<AsyncUtils.GetSolanaEstimatedTxFeeResponseContext> estimatesContexts =
                new ArrayList<AsyncUtils.GetSolanaEstimatedTxFeeResponseContext>();

        for (TransactionInfo txInfo : solanaTransactions) {
            AsyncUtils.GetSolanaEstimatedTxFeeResponseContext estimatesContext =
                    new AsyncUtils.GetSolanaEstimatedTxFeeResponseContext(
                            estimatesMultiResponse.singleResponseComplete);

            estimatesContext.txMetaId = txInfo.id;
            estimatesContexts.add(estimatesContext);

            if (mActivity.get() != null)
                mActivity
                        .get()
                        .getSolanaTxManagerProxy()
                        .getSolanaTxFeeEstimation(txInfo.chainId, txInfo.id, estimatesContext);
        }

        estimatesMultiResponse.setWhenAllCompletedAction(
                () -> {
                    for (AsyncUtils.GetSolanaEstimatedTxFeeResponseContext estimatesContext :
                            estimatesContexts) {
                        if (estimatesContext.error != SolanaProviderError.SUCCESS) {
                            continue;
                        }
                        mPerTxFee.put(estimatesContext.txMetaId, estimatesContext.fee);
                    }

                    runWhenDone.run();
                });
    }
}
