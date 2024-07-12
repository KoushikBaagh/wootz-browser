/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */

package org.chromium.chrome.browser.crypto_wallet.listeners;

/**
 * Interface implemented by {@link org.chromium.chrome.browser.app.WootzActivity} in charge of
 * defining the navigation behavior for onboarding, biometric prompt, next page, and navigation
 * icons.
 */
public interface OnNextPage {
    void incrementPages(int pages);

    void showWallet();

    void gotoCreationPage();

    void gotoRestorePage(final boolean isOnboarding);

    void showCloseButton(final boolean show);

    void showBackButton(final boolean show);
}
