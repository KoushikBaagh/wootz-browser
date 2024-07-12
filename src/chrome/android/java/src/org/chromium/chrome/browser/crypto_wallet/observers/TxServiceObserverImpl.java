/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */

package org.chromium.chrome.browser.crypto_wallet.observers;

import org.chromium.brave_wallet.mojom.TransactionInfo;  // koushik change
import org.chromium.brave_wallet.mojom.TxServiceObserver; // koushik change
import org.chromium.mojo.system.MojoException;

public class TxServiceObserverImpl implements TxServiceObserver {
    public interface TxServiceObserverImplDelegate {
        default void onNewUnapprovedTx(TransactionInfo txInfo) {}
        default void onUnapprovedTxUpdated(TransactionInfo txInfo) {}
        default void onTransactionStatusChanged(TransactionInfo txInfo) {}
    }

    private TxServiceObserverImplDelegate mDelegate;

    public TxServiceObserverImpl(TxServiceObserverImplDelegate delegate) {
        mDelegate = delegate;
    }

    @Override
    public void onNewUnapprovedTx(TransactionInfo txInfo) {
        if (mDelegate == null) return;

        mDelegate.onNewUnapprovedTx(txInfo);
    }

    @Override
    public void onUnapprovedTxUpdated(TransactionInfo txInfo) {
        if (mDelegate == null) return;

        mDelegate.onUnapprovedTxUpdated(txInfo);
    }

    @Override
    public void onTransactionStatusChanged(TransactionInfo txInfo) {
        if (mDelegate == null) return;

        mDelegate.onTransactionStatusChanged(txInfo);
    }

    @Override
    public void onTxServiceReset() {}

    @Override
    public void close() {
        mDelegate = null;
    }

    @Override
    public void onConnectionError(MojoException e) {}

    public void destroy() {
        mDelegate = null;
    }
}
