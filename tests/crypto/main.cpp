// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstddef>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "crypto-tests.h"
#include "../io.h"

extern "C" {
  #include "crypto/crypto-defines.h"
}

using namespace std;
typedef hash_t chash;

bool operator!=(const elliptic_curve_scalar_t &a, const elliptic_curve_scalar_t &b)
{
  return 0 != memcmp(&a, &b, sizeof(elliptic_curve_scalar_t));
}

bool operator!=(const elliptic_curve_point_t &a, const elliptic_curve_point_t &b)
{
  return 0 != memcmp(&a, &b, sizeof(elliptic_curve_point_t));
}

bool operator!=(const key_derivation_t &a, const key_derivation_t &b)
{
  return 0 != memcmp(&a, &b, sizeof(key_derivation_t));
}

int main(int argc, char *argv[])
{
  fstream input;
  string cmd;
  size_t test = 0;
  bool error = false;
  setup_random();
  if (argc != 2)
  {
    cerr << "invalid arguments" << endl;
    return 1;
  }
  input.open(argv[1], ios_base::in);
  for (;;)
  {
    ++test;
    input.exceptions(ios_base::badbit);
    if (!(input >> cmd))
    {
      break;
    }
    input.exceptions(ios_base::badbit | ios_base::failbit | ios_base::eofbit);
    if (cmd == "check_scalar")
    {
      elliptic_curve_scalar_t scalar;
      bool expected, actual;
      get(input, scalar, expected);
      actual = check_scalar(scalar);
      if (expected != actual)
      {
        goto error;
      }
    }
    else if (cmd == "random_scalar")
    {
      elliptic_curve_scalar_t expected, actual;
      get(input, expected);
      random_scalar((uint8_t *)&actual);
      if (expected != actual)
      {
        goto error;
      }
    }
    else if (cmd == "hash_to_scalar")
    {
      vector<char> data;
      elliptic_curve_scalar_t expected, actual;
      get(input, data, expected);
      hash_to_scalar((const uint8_t *)data.data(), data.size(), (uint8_t *)&actual);
      if (expected != actual)
      {
        goto error;
      }
    }
    else if (cmd == "generate_keys")
    {
      public_key_t expected1, actual1;
      secret_key_t expected2, actual2;
      get(input, expected1, expected2);
      generate_keys((uint8_t *) &actual1, (uint8_t *) &actual2);
      if (expected1 != actual1 || expected2 != actual2)
      {
        goto error;
      }
    }
    else if (cmd == "check_key")
    {
      public_key_t key;
      bool expected, actual;
      get(input, key, expected);
      actual = check_key((uint8_t*)&key);
      if (expected != actual)
      {
        goto error;
      }
    }
    else if (cmd == "secret_key_to_public_key")
    {
      secret_key_t sec;
      bool expected1, actual1;
      public_key_t expected2, actual2;
      get(input, sec, expected1);
      if (expected1)
      {
        get(input, expected2);
      }
      actual1 = secret_key_to_public_key((const uint8_t*)&sec, (uint8_t*)&actual2);
      if (expected1 != actual1 || (expected1 && expected2 != actual2))
      {
        goto error;
      }
    }
    else if (cmd == "generate_key_derivation")
    {
      public_key_t key1;
      secret_key_t key2;
      bool expected1, actual1;
      key_derivation_t expected2, actual2;
      get(input, key1, key2, expected1);
      if (expected1)
      {
        get(input, expected2);
      }
      actual1 = generate_key_derivation((const uint8_t*)&key1, (const uint8_t*)&key2, (uint8_t*)&actual2);
      if (expected1 != actual1 || (expected1 && expected2 != actual2))
      {
        goto error;
      }
    }
    else if (cmd == "derive_public_key")
    {
      key_derivation_t derivation;
      size_t output_index;
      public_key_t base;
      bool expected1, actual1;
      public_key_t expected2, actual2;
      get(input, derivation, output_index, base, expected1);
      if (expected1)
      {
        get(input, expected2);
      }
      actual1 = derive_public_key((const uint8_t*)&derivation, output_index, (const uint8_t*)&base, (uint8_t*)&actual2);
      if (expected1 != actual1 || (expected1 && expected2 != actual2))
      {
        goto error;
      }
    }
    else if (cmd == "derive_secret_key")
    {
      key_derivation_t derivation;
      size_t output_index;
      secret_key_t base;
      secret_key_t expected, actual;
      get(input, derivation, output_index, base, expected);
      derive_secret_key((const uint8_t*)&derivation, output_index, (const uint8_t*)&base, (uint8_t*)&actual);
      if (expected != actual)
      {
        goto error;
      }
    }
    else if (cmd == "underive_public_key")
    {
      key_derivation_t derivation;
      size_t output_index;
      public_key_t derived_key;
      bool expected1, actual1;
      public_key_t expected2, actual2;
      get(input, derivation, output_index, derived_key, expected1);
      if (expected1)
      {
        get(input, expected2);
      }
      actual1 = underive_public_key((const uint8_t *)&derivation, output_index, (const uint8_t *)&derived_key, (uint8_t *)&actual2);
      if (expected1 != actual1 || (expected1 && expected2 != actual2))
      {
        goto error;
      }
    }
    else if (cmd == "generate_signature")
    {
      chash prefix_hash;
      public_key_t pub;
      secret_key_t sec;
      signature_t expected, actual;
      get(input, prefix_hash, pub, sec, expected);
      generate_signature((const uint8_t *)&prefix_hash, (const uint8_t *)&pub, (const uint8_t *)&sec, (uint8_t *)&actual);
      if (expected != actual)
      {
        goto error;
      }
    }
    else if (cmd == "check_signature")
    {
      chash prefix_hash;
      public_key_t pub;
      signature_t sig;
      bool expected, actual;
      get(input, prefix_hash, pub, sig, expected);
      actual = check_signature((const uint8_t *)&prefix_hash, (const uint8_t *)&pub, (const uint8_t *)&sig);
      if (expected != actual)
      {
        goto error;
      }
    }
    else if (cmd == "hash_to_point")
    {
      chash h;
      elliptic_curve_point_t expected, actual;
      get(input, h, expected);
      hash_to_point(h, actual);
      if (expected != actual)
      {
        goto error;
      }
    }
    else if (cmd == "hash_to_ec")
    {
      public_key_t key;
      elliptic_curve_point_t expected, actual;
      get(input, key, expected);
      hash_to_ec(key, actual);
      if (expected != actual)
      {
        goto error;
      }
    }
    else if (cmd == "generate_key_image")
    {
      public_key_t pub;
      secret_key_t sec;
      key_image_t expected, actual;
      get(input, pub, sec, expected);
      generate_key_image((const uint8_t *)&pub, (const uint8_t *)&sec, (uint8_t *)&actual);
      if (expected != actual)
      {
        goto error;
      }
    }
    else if (cmd == "generate_ring_signature")
    {
      chash prefix_hash;
      key_image_t image;
      vector<public_key_t> vpubs;
      vector<const public_key_t *> pubs;
      size_t pubs_count;
      secret_key_t sec;
      size_t sec_index;
      vector<signature_t> expected, actual;
      size_t i;
      get(input, prefix_hash, image, pubs_count);
      vpubs.resize(pubs_count);
      pubs.resize(pubs_count);
      for (i = 0; i < pubs_count; i++)
      {
        get(input, vpubs[i]);
        pubs[i] = &vpubs[i];
      }
      get(input, sec, sec_index);
      expected.resize(pubs_count);
      getvar(input, pubs_count * sizeof(signature_t), expected.data());
      actual.resize(pubs_count);
      generate_ring_signature(prefix_hash, image, pubs.data(), pubs_count, sec, sec_index, actual.data());
      if (expected != actual)
      {
        goto error;
      }
    }
    else if (cmd == "check_ring_signature")
    {
      chash prefix_hash;
      key_image_t image;
      vector<public_key_t> vpubs;
      vector<const public_key_t *> pubs;
      size_t pubs_count;
      vector<signature_t> sigs;
      bool expected, actual;
      size_t i;
      get(input, prefix_hash, image, pubs_count);
      vpubs.resize(pubs_count);
      pubs.resize(pubs_count);
      for (i = 0; i < pubs_count; i++)
      {
        get(input, vpubs[i]);
        pubs[i] = &vpubs[i];
      }
      sigs.resize(pubs_count);
      getvar(input, pubs_count * sizeof(signature_t), sigs.data());
      get(input, expected);
      actual = check_ring_signature(prefix_hash, image, pubs.data(), pubs_count, sigs.data());
      if (expected != actual)
      {
        goto error;
      }
    }
    else
    {
      throw ios_base::failure("Unknown function: " + cmd);
    }
    continue;
  error:
    cerr << "Wrong result on test " << test << endl;
    error = true;
  }
  return error ? 1 : 0;
}
