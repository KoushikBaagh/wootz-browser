/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */

package org.chromium.chrome.browser.crypto_wallet.listeners;

import androidx.annotation.NonNull;

import org.chromium.chrome.browser.crypto_wallet.model.AccountSelectorItemModel;

public interface AccountSelectorItemListener {
    void onAccountClick(@NonNull final AccountSelectorItemModel accountSelectorItemModel);
}
