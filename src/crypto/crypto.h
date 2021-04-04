
#ifndef CRYPTO_H_
#define CRYPTO_H_
#include <stdint.h>
#include "types.h"

#define VARINT_WRITE(DEST, INT)            \
  while ((INT) >= 0x80)                    \
  {                                        \
    *(DEST)++ = ((char)(INT)&0x7f) | 0x80; \
    (INT) >>= 7;                           \
  }                                        \
  *DEST++ = (char)(INT);

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
extern void hash_data_to_ec(const uint8_t *data, size_t len, uint8_t *key);

extern void generate_signature(const uint8_t *prefix_hash, const uint8_t *pub, const uint8_t *sec, uint8_t *sig);
extern int check_signature(const uint8_t *prefix_hash, const uint8_t *pub, const uint8_t *sig);

extern void hash_to_ec(const uint8_t *key, uint8_t *res);
extern void generate_key_image(const uint8_t *pub, const uint8_t *sec, uint8_t *image);

extern void generate_ring_signature(const uint8_t *prefix_hash, const uint8_t *image,
                                    const uint8_t *const *pubs, size_t pubs_count,
                                    const uint8_t *sec, size_t sec_index,
                                    uint8_t *sig);
extern int check_ring_signature(const uint8_t *prefix_hash, const uint8_t *image,
                                const uint8_t *const *pubs, size_t pubs_count,
                                const uint8_t *sig);

extern void hash_to_point(const uint8_t *hash, uint8_t *point);
extern void hash_to_ec_ex(const uint8_t *hash, uint8_t *ec);

#endif