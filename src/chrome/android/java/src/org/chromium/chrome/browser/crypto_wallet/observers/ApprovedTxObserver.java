/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */
package org.chromium.chrome.browser.crypto_wallet.observers;

public interface ApprovedTxObserver {
    void onTxApprovedRejected(final boolean approved, final String txId);
    void onTxPending(final String txId);
}
