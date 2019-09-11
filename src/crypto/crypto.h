// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstddef>
#include <limits>
#include <mutex>
#include <type_traits>
#include <vector>

#include <crypto.h>

#include "generic-ops.h"
#include "hash.h"

extern "C"
{
#include "random.h"
#include "crypto-defines.h"
}

namespace crypto
{

extern std::mutex random_lock;

/* Generate a value filled with random bytes.
   */
template <typename T>
typename std::enable_if<std::is_pod<T>::value, T>::type rand()
{
  typename std::remove_cv<T>::type res;
  std::lock_guard<std::mutex> lock(random_lock);
  generate_random_bytes_not_thread_safe(sizeof(T), &res);
  return res;
}

/* Random number engine based on crypto::rand()
   */
template <typename T>
class random_engine
{
public:
  typedef T result_type;

#ifdef __clang__
  constexpr static T min()
  {
    return (std::numeric_limits<T>::min)();
  }

  constexpr static T max()
  {
    return (std::numeric_limits<T>::max)();
  }
#else
  static T(min)()
  {
    return (std::numeric_limits<T>::min)();
  }

  static T(max)()
  {
    return (std::numeric_limits<T>::max)();
  }
#endif
  typename std::enable_if<std::is_unsigned<T>::value, T>::type operator()()
  {
    return rand<T>();
  }
};

/* To generate an ephemeral key used to send money to:
   * * The sender generates a new key pair, which becomes the transaction key. The public transaction key is included in "extra" field.
   * * Both the sender and the receiver generate key derivation from the transaction key and the receivers' "view" key.
   * * The sender uses key derivation, the output index, and the receivers' "spend" key to derive an ephemeral public key.
   * * The receiver can either derive the public key (to check that the transaction is addressed to him) or the private key (to spend the money).
   */

bool derive_public_key(const key_derivation_t &derivation, size_t output_index, const public_key_t &base, public_key_t &derived_key);
bool derive_public_key_suffix(const key_derivation_t &derivation, size_t output_index, const public_key_t &base, const uint8_t *suffix, size_t suffixLength, public_key_t &derived_key);

void derive_secret_key(const key_derivation_t &derivation, size_t output_index, const secret_key_t &base, secret_key_t &derived_key);
void derive_secret_key_suffix(const key_derivation_t &derivation, size_t output_index, const secret_key_t &base, const uint8_t *suffix, size_t suffixLength, secret_key_t &derived_key);

bool underive_public_key_and_get_scalar(const key_derivation_t &derivation, std::size_t output_index,
                                        const public_key_t &derived_key, public_key_t &base, elliptic_curve_scalar_t &hashed_derivation);
bool underive_public_key_suffix(const key_derivation_t &derivation, size_t output_index,
                                const public_key_t &derived_key, const uint8_t *prefix, size_t prefixLength, public_key_t &base);

bool underive_public_key(const key_derivation_t &derivation, size_t output_index,
                         const public_key_t &derived_key, public_key_t &base);

void generate_signature(const hash_t &prefix_hash, const public_key_t &pub, const secret_key_t &sec, signature_t &sig);

bool check_signature(const hash_t &prefix_hash, const public_key_t &pub, const signature_t &sig);

/* To send money to a key:
   * * The sender generates an ephemeral key and includes it in transaction output.
   * * To spend the money, the receiver generates a key image from it.
   * * Then he selects a bunch of outputs, including the one he spends, and uses them to generate a ring signature.
   * To check the signature, it is necessary to collect all the keys that were used to generate it. To detect double spends, it is necessary to check that each key image is used at most once.
   */
void generate_key_image(const public_key_t &pub, const secret_key_t &sec, key_image_t &image);

void hash_data_to_ec(const uint8_t *data, std::size_t len, public_key_t &key);

void generate_ring_signature(const hash_t &prefix_hash, const key_image_t &image, const public_key_t *const *pubs, std::size_t pubs_count, const secret_key_t &sec, std::size_t sec_index, signature_t *sig);

bool check_ring_signature(const hash_t &prefix_hash, const key_image_t &image, const public_key_t *const *pubs, size_t pubs_count, const signature_t *sig);

} // namespace crypto

CRYPTO_MAKE_HASHABLE(public_key_t)
CRYPTO_MAKE_HASHABLE(key_image_t)
CRYPTO_MAKE_COMPARABLE(signature_t)
CRYPTO_MAKE_COMPARABLE(secret_key_t)
