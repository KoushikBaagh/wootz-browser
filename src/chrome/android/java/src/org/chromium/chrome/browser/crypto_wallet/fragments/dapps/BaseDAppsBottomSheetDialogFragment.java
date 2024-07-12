/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */

package org.chromium.chrome.browser.crypto_wallet.fragments.dapps;

import android.app.Activity;

import org.chromium.brave_wallet.mojom.BraveWalletService;  // koushik change
import org.chromium.brave_wallet.mojom.JsonRpcService; // koushik change
import org.chromium.brave_wallet.mojom.KeyringService; // koushik change
import org.chromium.chrome.browser.crypto_wallet.activities.BraveWalletBaseActivity; // koushik change
import org.chromium.chrome.browser.crypto_wallet.fragments.WalletBottomSheetDialogFragment;

public class BaseDAppsBottomSheetDialogFragment extends WalletBottomSheetDialogFragment {
    public WootzWalletService getWootzWalletService() {
        Activity activity = getActivity();
        if (activity instanceof WootzWalletBaseActivity) {
            return ((WootzWalletBaseActivity) activity).getWootzWalletService();
        }

        return null;
    }

    public KeyringService getKeyringService() {
        Activity activity = getActivity();
        if (activity instanceof WootzWalletBaseActivity) {
            return ((WootzWalletBaseActivity) activity).getKeyringService();
        }

        return null;
    }

    public JsonRpcService getJsonRpcService() {
        Activity activity = getActivity();
        if (activity instanceof WootzWalletBaseActivity) {
            return ((WootzWalletBaseActivity) activity).getJsonRpcService();
        }

        return null;
    }
}
