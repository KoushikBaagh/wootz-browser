/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */

package org.chromium.chrome.browser.crypto_wallet.fragments;

import android.content.DialogInterface;

import androidx.annotation.NonNull;

import com.google.android.material.bottomsheet.BottomSheetDialogFragment;

import org.chromium.chrome.browser.app.domain.KeyringModel;
import org.chromium.chrome.browser.crypto_wallet.observers.KeyringServiceObserverImpl;

/**
 * Base class for {@code BottomSheetDialogFragment} with wallet specific implementation
 * (auto-dismiss when locked, clean up etc).
 */
public class WalletBottomSheetDialogFragment extends BottomSheetDialogFragment
        implements KeyringServiceObserverImpl.KeyringServiceObserverImplDelegate {
    private KeyringServiceObserverImpl mKeyringObserver;

    @Override
    public void onDismiss(@NonNull DialogInterface dialog) {
        super.onDismiss(dialog);
        if (mKeyringObserver != null) {
            mKeyringObserver.close();
        }
    }

    protected void registerKeyringObserver(KeyringModel keyringModel) {
        if (keyringModel == null || mKeyringObserver != null) return;

        mKeyringObserver = new KeyringServiceObserverImpl(this);
        keyringModel.registerKeyringObserver(mKeyringObserver);
    }

    @Override
    public void locked() {
        dismiss();
    }
}
