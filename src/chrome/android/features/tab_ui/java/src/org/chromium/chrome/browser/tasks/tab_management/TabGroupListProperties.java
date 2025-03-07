// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tasks.tab_management;

import org.chromium.ui.modelutil.PropertyKey;
import org.chromium.ui.modelutil.PropertyModel.WritableBooleanPropertyKey;

/** Properties for displaying a single tab group row. */
public class TabGroupListProperties {
    public static final WritableBooleanPropertyKey EMPTY_STATE_VISIBLE =
            new WritableBooleanPropertyKey();
    public static final WritableBooleanPropertyKey SYNC_ENABLED = new WritableBooleanPropertyKey();

    public static final PropertyKey[] ALL_KEYS = {EMPTY_STATE_VISIBLE, SYNC_ENABLED};
}
