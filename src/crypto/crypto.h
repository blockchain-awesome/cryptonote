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

namespace crypto {

  extern "C" {
    #include "random.h"
    #include "crypto-defines.h"
  }

  extern std::mutex random_lock;

  class crypto_ops {
    crypto_ops();
    crypto_ops(const crypto_ops &);
    void operator=(const crypto_ops &);
    ~crypto_ops();

    static void generate_keys(public_key_t &, secret_key_t &);
    friend void generate_keys(public_key_t &, secret_key_t &);
    static bool check_key(const public_key_t &);
    friend bool check_key(const public_key_t &);
    static bool secret_key_to_public_key(const secret_key_t &, public_key_t &);
    friend bool secret_key_to_public_key(const secret_key_t &, public_key_t &);
    static bool generate_key_derivation(const public_key_t &, const secret_key_t &, key_derivation_t &);
    friend bool generate_key_derivation(const public_key_t &, const secret_key_t &, key_derivation_t &);
    static bool derive_public_key(const key_derivation_t &, size_t, const public_key_t &, public_key_t &);
    friend bool derive_public_key(const key_derivation_t &, size_t, const public_key_t &, public_key_t &);
    friend bool derive_public_key(const key_derivation_t &, size_t, const public_key_t &, const uint8_t*, size_t, public_key_t &);
    static bool derive_public_key(const key_derivation_t &, size_t, const public_key_t &, const uint8_t*, size_t, public_key_t &);
    //hack for pg
    static bool underive_public_key_and_get_scalar(const key_derivation_t &, std::size_t, const public_key_t &, public_key_t &, elliptic_curve_scalar_t &);
    friend bool underive_public_key_and_get_scalar(const key_derivation_t &, std::size_t, const public_key_t &, public_key_t &, elliptic_curve_scalar_t &);
    static void generate_incomplete_key_image(const public_key_t &, elliptic_curve_point_t &);
    friend void generate_incomplete_key_image(const public_key_t &, elliptic_curve_point_t &);
    //
    static void derive_secret_key(const key_derivation_t &, size_t, const secret_key_t &, secret_key_t &);
    friend void derive_secret_key(const key_derivation_t &, size_t, const secret_key_t &, secret_key_t &);
    static void derive_secret_key(const key_derivation_t &, size_t, const secret_key_t &, const uint8_t*, size_t, secret_key_t &);
    friend void derive_secret_key(const key_derivation_t &, size_t, const secret_key_t &, const uint8_t*, size_t, secret_key_t &);
    static bool underive_public_key(const key_derivation_t &, size_t, const public_key_t &, public_key_t &);
    friend bool underive_public_key(const key_derivation_t &, size_t, const public_key_t &, public_key_t &);
    static bool underive_public_key(const key_derivation_t &, size_t, const public_key_t &, const uint8_t*, size_t, public_key_t &);
    friend bool underive_public_key(const key_derivation_t &, size_t, const public_key_t &, const uint8_t*, size_t, public_key_t &);
    static void generate_signature(const hash_t &, const public_key_t &, const secret_key_t &, signature_t &);
    friend void generate_signature(const hash_t &, const public_key_t &, const secret_key_t &, signature_t &);
    static bool check_signature(const hash_t &, const public_key_t &, const signature_t &);
    friend bool check_signature(const hash_t &, const public_key_t &, const signature_t &);
    static void generate_key_image(const public_key_t &, const secret_key_t &, key_image_t &);
    friend void generate_key_image(const public_key_t &, const secret_key_t &, key_image_t &);
    static void hash_data_to_ec(const uint8_t*, std::size_t, public_key_t&);
    friend void hash_data_to_ec(const uint8_t*, std::size_t, public_key_t&);
    static void generate_ring_signature(const hash_t &, const key_image_t &,
      const public_key_t *const *, size_t, const secret_key_t &, size_t, signature_t *);
    friend void generate_ring_signature(const hash_t &, const key_image_t &,
      const public_key_t *const *, size_t, const secret_key_t &, size_t, signature_t *);
    static bool check_ring_signature(const hash_t &, const key_image_t &,
      const public_key_t *const *, size_t, const signature_t *);
    friend bool check_ring_signature(const hash_t &, const key_image_t &,
      const public_key_t *const *, size_t, const signature_t *);
  };

  /* Generate a value filled with random bytes.
   */
  template<typename T>
  typename std::enable_if<std::is_pod<T>::value, T>::type rand() {
    typename std::remove_cv<T>::type res;
    std::lock_guard<std::mutex> lock(random_lock);
    generate_random_bytes_not_thread_safe(sizeof(T), &res);
    return res;
  }

  /* Random number engine based on crypto::rand()
   */
  template <typename T>
  class random_engine {
  public:
    typedef T result_type;

#ifdef __clang__
    constexpr static T min() {
      return (std::numeric_limits<T>::min)();
    }

    constexpr static T max() {
      return (std::numeric_limits<T>::max)();
    }
#else
    static T(min)() {
      return (std::numeric_limits<T>::min)();
    }

    static T(max)() {
      return (std::numeric_limits<T>::max)();
    }
#endif
    typename std::enable_if<std::is_unsigned<T>::value, T>::type operator()() {
      return rand<T>();
    }
  };

  /* Generate a new key pair
   */
  inline void generate_keys(public_key_t &pub, secret_key_t &sec) {
    crypto_ops::generate_keys(pub, sec);
  }

  /* Check a public key. Returns true if it is valid, false otherwise.
   */
  inline bool check_key(const public_key_t &key) {
    return crypto_ops::check_key(key);
  }

  /* Checks a private key and computes the corresponding public key.
   */
  inline bool secret_key_to_public_key(const secret_key_t &sec, public_key_t &pub) {
    return crypto_ops::secret_key_to_public_key(sec, pub);
  }

  /* To generate an ephemeral key used to send money to:
   * * The sender generates a new key pair, which becomes the transaction key. The public transaction key is included in "extra" field.
   * * Both the sender and the receiver generate key derivation from the transaction key and the receivers' "view" key.
   * * The sender uses key derivation, the output index, and the receivers' "spend" key to derive an ephemeral public key.
   * * The receiver can either derive the public key (to check that the transaction is addressed to him) or the private key (to spend the money).
   */
  inline bool generate_key_derivation(const public_key_t &key1, const secret_key_t &key2, key_derivation_t &derivation) {
    return crypto_ops::generate_key_derivation(key1, key2, derivation);
  }

  inline bool derive_public_key(const key_derivation_t &derivation, size_t output_index,
    const public_key_t &base, const uint8_t* prefix, size_t prefixLength, public_key_t &derived_key) {
    return crypto_ops::derive_public_key(derivation, output_index, base, prefix, prefixLength, derived_key);
  }

  inline bool derive_public_key(const key_derivation_t &derivation, size_t output_index,
    const public_key_t &base, public_key_t &derived_key) {
    return crypto_ops::derive_public_key(derivation, output_index, base, derived_key);
  }


  inline bool underive_public_key_and_get_scalar(const key_derivation_t &derivation, std::size_t output_index,
    const public_key_t &derived_key, public_key_t &base, elliptic_curve_scalar_t &hashed_derivation) {
    return crypto_ops::underive_public_key_and_get_scalar(derivation, output_index, derived_key, base, hashed_derivation);
  }
  
  inline void derive_secret_key(const key_derivation_t &derivation, std::size_t output_index,
    const secret_key_t &base, const uint8_t* prefix, size_t prefixLength, secret_key_t &derived_key) {
    crypto_ops::derive_secret_key(derivation, output_index, base, prefix, prefixLength, derived_key);
  }

  inline void derive_secret_key(const key_derivation_t &derivation, std::size_t output_index,
    const secret_key_t &base, secret_key_t &derived_key) {
    crypto_ops::derive_secret_key(derivation, output_index, base, derived_key);
  }


  /* Inverse function of derive_public_key. It can be used by the receiver to find which "spend" key was used to generate a transaction. This may be useful if the receiver used multiple addresses which only differ in "spend" key.
   */
  inline bool underive_public_key(const key_derivation_t &derivation, size_t output_index,
    const public_key_t &derived_key, const uint8_t* prefix, size_t prefixLength, public_key_t &base) {
    return crypto_ops::underive_public_key(derivation, output_index, derived_key, prefix, prefixLength, base);
  }

  inline bool underive_public_key(const key_derivation_t &derivation, size_t output_index,
    const public_key_t &derived_key, public_key_t &base) {
    return crypto_ops::underive_public_key(derivation, output_index, derived_key, base);
  }

  /* Generation and checking of a standard signature.
   */
  inline void generate_signature(const hash_t &prefix_hash, const public_key_t &pub, const secret_key_t &sec, signature_t &sig) {
    crypto_ops::generate_signature(prefix_hash, pub, sec, sig);
  }
  inline bool check_signature(const hash_t &prefix_hash, const public_key_t &pub, const signature_t &sig) {
    return crypto_ops::check_signature(prefix_hash, pub, sig);
  }

  /* To send money to a key:
   * * The sender generates an ephemeral key and includes it in transaction output.
   * * To spend the money, the receiver generates a key image from it.
   * * Then he selects a bunch of outputs, including the one he spends, and uses them to generate a ring signature.
   * To check the signature, it is necessary to collect all the keys that were used to generate it. To detect double spends, it is necessary to check that each key image is used at most once.
   */
  inline void generate_key_image(const public_key_t &pub, const secret_key_t &sec, key_image_t &image) {
    crypto_ops::generate_key_image(pub, sec, image);
  }

  inline void hash_data_to_ec(const uint8_t* data, std::size_t len, public_key_t& key) {
    crypto_ops::hash_data_to_ec(data, len, key);
  }

  inline void generate_ring_signature(const hash_t &prefix_hash, const key_image_t &image,
    const public_key_t *const *pubs, std::size_t pubs_count,
    const secret_key_t &sec, std::size_t sec_index,
    signature_t *sig) {
    crypto_ops::generate_ring_signature(prefix_hash, image, pubs, pubs_count, sec, sec_index, sig);
  }
  inline bool check_ring_signature(const hash_t &prefix_hash, const key_image_t &image,
    const public_key_t *const *pubs, size_t pubs_count,
    const signature_t *sig) {
    return crypto_ops::check_ring_signature(prefix_hash, image, pubs, pubs_count, sig);
  }

  /* Variants with vector<const public_key_t *> parameters.
   */
  inline void generate_ring_signature(const hash_t &prefix_hash, const key_image_t &image,
    const std::vector<const public_key_t *> &pubs,
    const secret_key_t &sec, size_t sec_index,
    signature_t *sig) {
    generate_ring_signature(prefix_hash, image, pubs.data(), pubs.size(), sec, sec_index, sig);
  }
  inline bool check_ring_signature(const hash_t &prefix_hash, const key_image_t &image,
    const std::vector<const public_key_t *> &pubs,
    const signature_t *sig) {
    return check_ring_signature(prefix_hash, image, pubs.data(), pubs.size(), sig);
  }

}

CRYPTO_MAKE_HASHABLE(public_key_t)
CRYPTO_MAKE_HASHABLE(key_image_t)
CRYPTO_MAKE_COMPARABLE(signature_t)
CRYPTO_MAKE_COMPARABLE(secret_key_t)
