/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */

package org.chromium.chrome.browser.crypto_wallet.fragments;

import android.content.Context;

import androidx.fragment.app.Fragment;

import org.chromium.brave_wallet.mojom.SolanaInstruction;  // koushik change
import org.chromium.brave_wallet.mojom.TransactionInfo; // koushik change
import org.chromium.chrome.browser.crypto_wallet.adapters.TwoLineItemRecyclerViewAdapter;
import org.chromium.chrome.browser.crypto_wallet.presenters.SolanaInstructionPresenter;
import org.chromium.chrome.browser.crypto_wallet.util.TransactionUtils;

import java.util.ArrayList;
import java.util.List;

public class SolanaTxDetailsFragment {
    public static Fragment newInstance(TransactionInfo txInfo, Context context) {
        List<TwoLineItemRecyclerViewAdapter.TwoLineItem> details = new ArrayList<>();
        SolanaInstruction[] instructions =
                TransactionUtils.safeSolData(txInfo.txDataUnion).instructions;
        details.add(new TwoLineItemRecyclerViewAdapter.TwoLineItemText(
                context.getString(TransactionUtils.getTxType(txInfo))));

        for (SolanaInstruction solanaInstruction : instructions) {
            SolanaInstructionPresenter solanaInstructionPresenter =
                    new SolanaInstructionPresenter(solanaInstruction);
            details.addAll(solanaInstructionPresenter.toTwoLineList(context));
            if (instructions.length > 1) {
                details.add(new TwoLineItemRecyclerViewAdapter.TwoLineItemDivider());
            }
        }
        return new TwoLineItemFragment(details);
    }
}
