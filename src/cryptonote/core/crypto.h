#pragma once

#include "crypto/types.h"
#include <stddef.h>

extern "C"
{
extern void random_scalar(uint8_t *res);
extern void hash_to_scalar(const uint8_t *scalar, size_t length, uint8_t *hash);
extern void generate_keys(uint8_t *public_key, uint8_t *secret_key);

extern int check_key(const uint8_t *public_key);
extern int secret_key_to_public_key(const uint8_t *secret_key, uint8_t *public_key);
extern int generate_key_derivation(const uint8_t *public_key, const uint8_t *secret_key, uint8_t *key_derivation);
extern int derive_public_key(const uint8_t *derivation, size_t output_index,
                        const uint8_t *base, uint8_t *derived_key);
extern int derive_public_key_suffix(const uint8_t *derivation, size_t output_index,
                               const uint8_t *base, const uint8_t *suffix, size_t suffixLength, uint8_t *derived_key);
extern void derive_secret_key(const uint8_t *derivation, size_t output_index, const uint8_t *base, uint8_t *derived_key);
extern void derive_secret_key_suffix(const uint8_t *derivation, size_t output_index, const uint8_t *base, const uint8_t *suffix, size_t suffixLength, uint8_t *derived_key);

extern int underive_public_key(const uint8_t *derivation, size_t output_index,
                          const uint8_t *derived_key, uint8_t *base);
extern int underive_public_key_suffix(const uint8_t *derivation, size_t output_index,
                                 const uint8_t *derived_key, const uint8_t *suffix, size_t suffixLength, uint8_t *base);

extern int underive_public_key_and_get_scalar(const uint8_t *derivation, size_t output_index,
                                         const uint8_t *derived_key, uint8_t *base, uint8_t *hashed_derivation);

  /* To generate an ephemeral key used to send money to:
   * * The sender generates a new key pair, which becomes the transaction key. The public transaction key is included in "extra" field.
   * * Both the sender and the receiver generate key derivation from the transaction key and the receivers' "view" key.
   * * The sender uses key derivation, the output index, and the receivers' "spend" key to derive an ephemeral public key.
   * * The receiver can either derive the public key (to check that the transaction is addressed to him) or the private key (to spend the money).
   */

extern void generate_signature(const uint8_t *prefix_hash, const uint8_t *pub, const uint8_t *sec, uint8_t *sig);

extern int check_signature(const uint8_t *prefix_hash, const uint8_t *pub, const uint8_t *sig);

  /* To send money to a key:
   * * The sender generates an ephemeral key and includes it in transaction output.
   * * To spend the money, the receiver generates a key image from it.
   * * Then he selects a bunch of outputs, including the one he spends, and uses them to generate a ring signature.
   * To check the signature, it is necessary to collect all the keys that were used to generate it. To detect double spends, it is necessary to check that each key image is used at most once.
   */
extern void generate_key_image(const uint8_t *pub, const uint8_t *sec, uint8_t *image);

extern void hash_data_to_ec(const uint8_t *data, size_t len, uint8_t *key);

extern void generate_ring_signature(const uint8_t *prefix_hash, const uint8_t *image,
                             const uint8_t *const *pubs, size_t pubs_count,
                             const uint8_t *sec, size_t sec_index,
                             uint8_t *sig);
extern int check_ring_signature(const uint8_t *prefix_hash, const uint8_t *image,
                           const uint8_t *const *pubs, size_t pubs_count,
                           const uint8_t *sig);
}
