// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "cryptonote/crypto/crypto.cpp"
#include "crypto-tests.h"
#include "crypto/crypto-ops.h"

using namespace crypto;

bool check_scalar(const elliptic_curve_scalar_t &scalar) {
  return sc_check(reinterpret_cast<const unsigned char*>(&scalar)) == 0;
}

void hash_to_point(const hash_t &h, elliptic_curve_point_t &res) {
  ge_p2 point;
  ge_fromfe_frombytes_vartime(&point, reinterpret_cast<const unsigned char *>(&h));
  ge_tobytes(reinterpret_cast<unsigned char*>(&res), &point);
}

void hash_to_ec(const public_key_t &key, elliptic_curve_point_t &res) {
  ge_p3 tmp;
  hash_to_ec((const uint8_t *)&key, (uint8_t *)&tmp);
  ge_p3_tobytes(reinterpret_cast<unsigned char*>(&res), &tmp);
}
