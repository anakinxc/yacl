// Copyright 2019 Ant Group Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <memory>

#include "yacl/crypto/base/key_utils.h"
#include "yacl/crypto/base/pke/asymmetric_crypto.h"

namespace yacl::crypto {

// SM2
class Sm2Encryptor : public AsymmetricEncryptor {
 public:
  explicit Sm2Encryptor(openssl::UniquePkey&& pk) : pk_(std::move(pk)) {}
  explicit Sm2Encryptor(ByteContainerView pk_buf)
      : pk_(LoadKeyFromBuf(pk_buf)) {}

  AsymCryptoSchema GetSchema() const override { return schema_; }
  std::vector<uint8_t> Encrypt(ByteContainerView plaintext) override;

 private:
  const openssl::UniquePkey pk_;
  const AsymCryptoSchema schema_ = AsymCryptoSchema::SM2;
};

class Sm2Decryptor : public AsymmetricDecryptor {
 public:
  explicit Sm2Decryptor(openssl::UniquePkey&& sk) : sk_(std::move(sk)) {}
  explicit Sm2Decryptor(ByteContainerView sk_buf)
      : sk_(LoadKeyFromBuf(sk_buf)) {}

  AsymCryptoSchema GetSchema() const override { return schema_; }
  std::vector<uint8_t> Decrypt(ByteContainerView ciphertext) override;

 private:
  const openssl::UniquePkey sk_;
  const AsymCryptoSchema schema_ = AsymCryptoSchema::SM2;
};

}  // namespace yacl::crypto