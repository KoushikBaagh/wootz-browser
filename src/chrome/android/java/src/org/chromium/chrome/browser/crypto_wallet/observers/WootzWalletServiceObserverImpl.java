/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */

package org.chromium.chrome.browser.crypto_wallet.observers;

import org.chromium.brave_wallet.mojom.BlockchainToken;  // koushik change
import org.chromium.brave_wallet.mojom.BraveWalletServiceObserver;  // koushik change
import org.chromium.brave_wallet.mojom.OriginInfo; // koushik change
import org.chromium.mojo.system.MojoException;

import java.lang.ref.WeakReference;

public class WootzWalletServiceObserverImpl implements WootzWalletServiceObserver {
    public interface WootzWalletServiceObserverImplDelegate {
        default void onActiveOriginChanged(OriginInfo originInfo) {}

        default void onDefaultEthereumWalletChanged(int wallet) {}

        default void onDefaultSolanaWalletChanged(int wallet) {}

        default void onDefaultBaseCurrencyChanged(String currency) {}

        default void onDefaultBaseCryptocurrencyChanged(String cryptocurrency) {}

        default void onNetworkListChanged() {}

        default void onDiscoverAssetsStarted() {}

        default void onDiscoverAssetsCompleted(BlockchainToken[] discoveredAssets) {}

        default void onResetWallet() {}
    }

    private WeakReference<WootzWalletServiceObserverImplDelegate> mDelegate;

    public WootzWalletServiceObserverImpl(WootzWalletServiceObserverImplDelegate delegate) {
        mDelegate = new WeakReference<>(delegate);
    }

    @Override
    public void onActiveOriginChanged(OriginInfo originInfo) {
        if (isActive()) getRef().onActiveOriginChanged(originInfo);
    }

    @Override
    public void onDefaultEthereumWalletChanged(int wallet) {
        if (isActive()) getRef().onDefaultEthereumWalletChanged(wallet);
    }

    @Override
    public void onDefaultSolanaWalletChanged(int wallet) {
        if (isActive()) getRef().onDefaultSolanaWalletChanged(wallet);
    }

    @Override
    public void onDefaultBaseCurrencyChanged(String currency) {
        if (isActive()) getRef().onDefaultBaseCurrencyChanged(currency);
    }

    @Override
    public void onDefaultBaseCryptocurrencyChanged(String cryptocurrency) {
        if (isActive()) getRef().onDefaultBaseCryptocurrencyChanged(cryptocurrency);
    }

    @Override
    public void onNetworkListChanged() {
        if (isActive()) getRef().onNetworkListChanged();
    }

    @Override
    public void onDiscoverAssetsStarted() {}

    @Override
    public void onDiscoverAssetsCompleted(BlockchainToken[] discoveredAssets) {
        if (isActive()) getRef().onDiscoverAssetsCompleted(discoveredAssets);
    }

    @Override
    public void onResetWallet() {
        if (isActive()) getRef().onResetWallet();
    }

    @Override
    public void close() {
        mDelegate.clear();
        mDelegate = null;
    }

    @Override
    public void onConnectionError(MojoException e) {}

    public void destroy() {
        close();
    }

    private WootzWalletServiceObserverImplDelegate getRef() {
        return mDelegate.get();
    }

    private boolean isActive() {
        return mDelegate != null && mDelegate.get() != null;
    }
}
