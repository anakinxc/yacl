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

#include "yacl/crypto/base/sign/sm2_signing.h"

#include "yacl/crypto/base/hash/hash_utils.h"

namespace yacl::crypto {

namespace {
// The default sm2 id. see:
// http://www.gmbz.org.cn/main/viewfile/2018011001400692565.html
constexpr std::string_view kDefaultSm2Id = {"1234567812345678"};
}  // namespace

std::vector<uint8_t> Sm2Signer::Sign(ByteContainerView message) const {
  // SM2 signatures can be generated by using the ‘DigestSign’ series of APIs,
  // for instance, EVP_DigestSignInit(), EVP_DigestSignUpdate() and
  // EVP_DigestSignFinal(). Ditto for the verification process by calling the
  // ‘DigestVerify’ series of APIs.
  //
  // That is, EVP_PKEY_sign() and EVP_PKEY_verify() does not work on sm2
  //
  // see: https://www.openssl.org/docs/man3.0/man7/EVP_PKEY-SM2.html
  auto ctx = openssl::UniquePkeyCtx(
      EVP_PKEY_CTX_new(sk_.get(), /* engine = default */ nullptr));
  YACL_ENFORCE(ctx != nullptr);
  EVP_PKEY_CTX_set1_id(ctx.get(), kDefaultSm2Id.data(), kDefaultSm2Id.size());

  // create message digest context
  auto mctx = openssl::UniqueMdCtx(EVP_MD_CTX_new());
  YACL_ENFORCE(mctx != nullptr);
  EVP_MD_CTX_set_pkey_ctx(mctx.get(), ctx.get());  // set it related to pkey ctx

  // init sign
  YACL_ENFORCE(EVP_DigestSignInit(
                   mctx.get(), /* pkey ctx has already been inited */ nullptr,
                   EVP_sm3(),
                   /* engine */ nullptr, sk_.get()) > 0);

  // write hashes of message into mctx
  YACL_ENFORCE(
      EVP_DigestSignUpdate(mctx.get(), message.data(), message.size()) > 0);

  // get output size
  size_t outlen = 0;
  YACL_ENFORCE(EVP_DigestSignFinal(mctx.get(), nullptr, &outlen) > 0);

  std::vector<uint8_t> out(outlen);
  YACL_ENFORCE(EVP_DigestSignFinal(mctx.get(), out.data(), &outlen) > 0);

  // Correct the signature size (this is necessary! TODO: find out why)
  out.resize(outlen);

  return out;
}

bool Sm2Verifier::Verify(ByteContainerView message,
                         ByteContainerView signature) const {
  auto ctx = openssl::UniquePkeyCtx(
      EVP_PKEY_CTX_new(pk_.get(), /* engine = default */ nullptr));
  YACL_ENFORCE(ctx != nullptr);
  EVP_PKEY_CTX_set1_id(ctx.get(), kDefaultSm2Id.data(), kDefaultSm2Id.size());

  // create message digest context
  auto mctx = openssl::UniqueMdCtx(EVP_MD_CTX_new());
  YACL_ENFORCE(mctx != nullptr);

  EVP_MD_CTX_set_pkey_ctx(mctx.get(), ctx.get());

  YACL_ENFORCE(EVP_DigestVerifyInit(
                   mctx.get(), /* pkey ctx has already been inited */ nullptr,
                   EVP_sm3(), /* engine */ nullptr, pk_.get()) > 0);

  YACL_ENFORCE(
      EVP_DigestVerifyUpdate(mctx.get(), message.data(), message.size()) > 0);

  int rc =
      EVP_DigestVerifyFinal(mctx.get(), signature.data(), signature.size());
  YACL_ENFORCE(rc >= 0);  // ret = 0, verify fail; ret = 1, verify success
  return rc == 1;
}

}  // namespace yacl::crypto