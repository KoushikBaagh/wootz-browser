/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */

package org.chromium.chrome.browser.crypto_wallet.model;

import org.chromium.brave_wallet.mojom.CoinType; // koushik change

public class WalletAccountCreationRequest {
    private @CoinType.EnumType int mCoinType;

    public WalletAccountCreationRequest(@CoinType.EnumType int coinType) {
        mCoinType = coinType;
    }

    public int getCoinType() {
        return mCoinType;
    }
}
