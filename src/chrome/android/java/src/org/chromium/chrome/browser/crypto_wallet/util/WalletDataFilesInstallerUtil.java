/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */

package org.chromium.chrome.browser.crypto_wallet.util;

import org.jni_zero.CalledByNative;

/**
 * Class that is used by wallet_data_files_installer.cc to determine, if we need to download
 * Wootz Wallet data files component on Android at startup
 */
public class WalletDataFilesInstallerUtil {
    @CalledByNative
    public static boolean isWootzWalletConfiguredOnAndroid() {
        return !Utils.shouldShowCryptoOnboarding();
    }
}
