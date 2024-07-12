/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */
package org.chromium.chrome.browser.crypto_wallet.util;

import org.chromium.brave_wallet.mojom.AccountId; // koushik change
import org.chromium.brave_wallet.mojom.AccountInfo; // koushik change
import org.chromium.brave_wallet.mojom.BraveWalletService; // koushik change

import java.util.Arrays;
import java.util.HashSet;
import java.util.stream.Collectors;

public class AccountsPermissionsHelper {
    private WootzWalletService mWootzWalletService;
    private AccountInfo[] mAccounts;
    private HashSet<AccountInfo> mAccountsWithPermissions;

    public AccountsPermissionsHelper(
            WootzWalletService wootzWalletService, AccountInfo[] accounts) {
        assert wootzWalletService != null;
        assert accounts != null;
        mWootzWalletService = wootzWalletService;
        mAccounts = accounts;
        mAccountsWithPermissions = new HashSet<AccountInfo>();
    }

    public HashSet<AccountInfo> getAccountsWithPermissions() {
        return mAccountsWithPermissions;
    }

    private static boolean containsAccount(AccountId[] accounts, AccountId searchFor) {
        return Arrays.stream(accounts).anyMatch(
                acc -> { return WalletUtils.accountIdsEqual(acc, searchFor); });
    }

    public void checkAccounts(Runnable runWhenDone) {
        AccountId[] allAccountIds =
                Arrays.stream(mAccounts).map(acc -> acc.accountId).toArray(AccountId[] ::new);
        mWootzWalletService.hasPermission(allAccountIds, (success, filteredAccounts) -> {
            mAccountsWithPermissions =
                    Arrays.stream(mAccounts)
                            .filter(acc -> containsAccount(filteredAccounts, acc.accountId))
                            .collect(Collectors.toCollection(HashSet::new));

            runWhenDone.run();
        });
    }
}
