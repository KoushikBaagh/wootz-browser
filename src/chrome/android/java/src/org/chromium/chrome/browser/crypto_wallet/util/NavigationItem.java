/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */

package org.chromium.chrome.browser.crypto_wallet.util;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

public class NavigationItem {
    @NonNull private final String mTitle;
    @NonNull private final Fragment mFragment;

    public NavigationItem(@NonNull final String title, @NonNull final Fragment fragment) {
        this.mTitle = title;
        this.mFragment = fragment;
    }

    @NonNull
    public String getTitle() {
        return mTitle;
    }

    @NonNull
    public Fragment getFragment() {
        return mFragment;
    }
}
