/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */
package org.chromium.chrome.browser.crypto_wallet.listeners;

public interface TransactionConfirmationListener {
    default void onNextTransaction(){};
    default void onApproveTransaction(){};
    default void onRejectTransaction(){};
    default void onRejectAllTransactions(){};
    default void onDismiss(){};
    default void onCancel(){};
}
