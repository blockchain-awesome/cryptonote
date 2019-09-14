// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#if defined(__cplusplus)
#include "cryptonote/crypto/crypto.h"
using namespace crypto;

extern "C" {
#endif

void setup_random(void);
void random_scalar(uint8_t *);
void hash_to_scalar(const uint8_t *data, size_t length, uint8_t *res);
void generate_keys(uint8_t *public_key, uint8_t *secret_key);

#if defined(__cplusplus)
}

bool check_scalar(const elliptic_curve_scalar_t &scalar);
void hash_to_point(const hash_t &h, elliptic_curve_point_t &res);
void hash_to_ec(const public_key_t &key, elliptic_curve_point_t &res);
#endif
