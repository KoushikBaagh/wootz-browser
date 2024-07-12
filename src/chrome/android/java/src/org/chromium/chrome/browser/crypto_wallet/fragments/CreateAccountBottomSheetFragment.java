/* Copyright (c) 2024 The WootzApp Authors. All rights reserved. */
package org.chromium.chrome.browser.crypto_wallet.fragments;

import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View; 
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.bottomsheet.BottomSheetDialogFragment;

import org.chromium.base.Log;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.app.BraveActivity; // koushik change
import org.chromium.chrome.browser.app.domain.WalletModel;
import org.chromium.chrome.browser.crypto_wallet.activities.AddAccountActivity;
import org.chromium.chrome.browser.crypto_wallet.adapters.CreateAccountAdapter;
import org.chromium.chrome.browser.crypto_wallet.model.CryptoAccountTypeInfo;

import java.util.ArrayList;
import java.util.List;

public class CreateAccountBottomSheetFragment extends BottomSheetDialogFragment
        implements CreateAccountAdapter.OnCreateAccountClickListener {
    public static final String TAG = "CreateAccount";
    private View rootView;
    private WalletModel mWalletModel;
    private List<CryptoAccountTypeInfo> mSupportedCryptoAccounts;
    private RecyclerView mRvAccounts;
    private CreateAccountAdapter mCreateAccountAdapter;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSupportedCryptoAccounts = new ArrayList<>();
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
            @Nullable Bundle savedInstanceState) {
        rootView = inflater.inflate(R.layout.fragment_create_account, container, false);
        return rootView;
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        mSupportedCryptoAccounts.clear();
        try {
            WootzActivity activity = WootzActivity.getWootzActivity();
            mWalletModel = activity.getWalletModel();
            mSupportedCryptoAccounts =
                    mWalletModel.getCryptoModel().getSupportedCryptoAccountTypes();
        } catch (WootzActivity.WootzActivityNotFoundException e) {
            Log.e(TAG, "onViewCreated " + e);
        }
        mRvAccounts = view.findViewById(R.id.fragment_create_account_rv);

        mCreateAccountAdapter =
                new CreateAccountAdapter(requireContext(), mSupportedCryptoAccounts);
        mRvAccounts.setAdapter(mCreateAccountAdapter);
        mCreateAccountAdapter.setOnAccountItemSelected(this);
    }

    @Override
    public void onAccountClick(CryptoAccountTypeInfo cryptoAccountTypeInfo) {
        Intent intent = AddAccountActivity.createIntentToAddAccount(
                getContext(), cryptoAccountTypeInfo.getCoinType());
        startActivity(intent);
        dismiss();
    }
}
