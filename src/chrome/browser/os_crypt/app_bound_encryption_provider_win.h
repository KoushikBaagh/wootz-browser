// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_OS_CRYPT_APP_BOUND_ENCRYPTION_PROVIDER_WIN_H_
#define CHROME_BROWSER_OS_CRYPT_APP_BOUND_ENCRYPTION_PROVIDER_WIN_H_

#include "components/os_crypt/async/browser/key_provider.h"

#include <optional>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "base/threading/sequence_bound.h"
#include "base/types/expected.h"
#include "base/win/windows_types.h"

class PrefService;
class PrefRegistrySimple;

namespace os_crypt_async {

class AppBoundEncryptionProviderWin : public os_crypt_async::KeyProvider {
 public:
  explicit AppBoundEncryptionProviderWin(PrefService* local_state);
  ~AppBoundEncryptionProviderWin() override;

  // Not copyable.
  AppBoundEncryptionProviderWin(const AppBoundEncryptionProviderWin&) = delete;
  AppBoundEncryptionProviderWin& operator=(
      const AppBoundEncryptionProviderWin&) = delete;

  static void RegisterLocalPrefs(PrefRegistrySimple* registry);

  // Set encryption enabled for testing. Should be called before creating any
  // instances of the class.
  static void SetEnableEncryptionForTesting(bool use_for_encryption);

 private:
  // These values are persisted to logs. Entries should not be renumbered and
  // numeric values should never be reused.
  enum class KeyRetrievalStatus {
    kSuccess = 0,
    kKeyNotFound = 1,
    kKeyDecodeFailure = 2,
    kInvalidKeyHeader = 3,
    kMaxValue = kInvalidKeyHeader,
  };

  // os_crypt_async::KeyProvider interface.
  void GetKey(KeyCallback callback) override;
  bool UseForEncryption() override;
  bool IsCompatibleWithOsCryptSync() override;

  base::expected<std::vector<const uint8_t>, KeyRetrievalStatus>
  RetrieveEncryptedKey();
  void StoreEncryptedKeyAndReply(
      const std::vector<uint8_t>& decrypted_key,
      KeyCallback callback,
      const std::optional<std::vector<const uint8_t>>& encrypted_key);
  static void ReplyWithKey(KeyCallback callback,
                           std::optional<std::vector<uint8_t>> decrypted_key);

  raw_ptr<PrefService> local_state_ GUARDED_BY_CONTEXT(sequence_checker_);

  class COMWorker;
  base::SequenceBound<COMWorker> com_worker_;

  SEQUENCE_CHECKER(sequence_checker_);

  base::WeakPtrFactory<AppBoundEncryptionProviderWin> weak_factory_{this};
};

}  // namespace os_crypt_async

#endif  // CHROME_BROWSER_OS_CRYPT_APP_BOUND_ENCRYPTION_PROVIDER_WIN_H_
