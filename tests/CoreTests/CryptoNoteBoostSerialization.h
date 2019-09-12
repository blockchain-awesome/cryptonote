// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/foreach.hpp>
#include <boost/serialization/is_bitwise_serializable.hpp>
#include "cryptonote/core/key.h"
#include "UnorderedContainersBoostSerialization.h"
#include "crypto/crypto.h"

//namespace cryptonote {
namespace boost
{
  namespace serialization
  {

  //---------------------------------------------------
  template <class Archive>
  inline void serialize(Archive &a, public_key_t &x, const boost::serialization::version_type ver)
  {
    a & reinterpret_cast<char (&)[sizeof(public_key_t)]>(x);
  }
  template <class Archive>
  inline void serialize(Archive &a, secret_key_t &x, const boost::serialization::version_type ver)
  {
    a & reinterpret_cast<char (&)[sizeof(secret_key_t)]>(x);
  }
  template <class Archive>
  inline void serialize(Archive &a, key_derivation_t &x, const boost::serialization::version_type ver)
  {
    a & reinterpret_cast<char (&)[sizeof(key_derivation_t)]>(x);
  }
  template <class Archive>
  inline void serialize(Archive &a, key_image_t &x, const boost::serialization::version_type ver)
  {
    a & reinterpret_cast<char (&)[sizeof(key_image_t)]>(x);
  }

  template <class Archive>
  inline void serialize(Archive &a, signature_t &x, const boost::serialization::version_type ver)
  {
    a & reinterpret_cast<char (&)[sizeof(signature_t)]>(x);
  }
  template <class Archive>
  inline void serialize(Archive &a, hash_t &x, const boost::serialization::version_type ver)
  {
    a & reinterpret_cast<char (&)[sizeof(hash_t)]>(x);
  }
  
  template <class Archive> void serialize(Archive& archive, cryptonote::multi_signature_input_t &output, unsigned int version) {
    archive & output.amount;
    archive & output.signatureCount;
    archive & output.outputIndex;
  }

  template <class Archive> void serialize(Archive& archive, cryptonote::multi_signature_output_t &output, unsigned int version) {
    archive & output.keys;
    archive & output.requiredSignatureCount;
  }

  template <class Archive>
  inline void serialize(Archive &a, cryptonote::key_output_t &x, const boost::serialization::version_type ver)
  {
    a & x.key;
  }

  template <class Archive>
  inline void serialize(Archive &a, cryptonote::base_input_t &x, const boost::serialization::version_type ver)
  {
    a & x.blockIndex;
  }

  template <class Archive>
  inline void serialize(Archive &a, cryptonote::key_input_t &x, const boost::serialization::version_type ver)
  {
    a & x.amount;
    a & x.outputIndexes;
    a & x.keyImage;
  }

  template <class Archive>
  inline void serialize(Archive &a, cryptonote::transaction_output_t &x, const boost::serialization::version_type ver)
  {
    a & x.amount;
    a & x.target;
  }


  template <class Archive>
  inline void serialize(Archive &a, cryptonote::transaction_t &x, const boost::serialization::version_type ver)
  {
    a & x.version;
    a & x.unlockTime;
    a & x.inputs;
    a & x.outputs;
    a & x.extra;
    a & x.signatures;
  }


  template <class Archive>
  inline void serialize(Archive &a, cryptonote::block_t &b, const boost::serialization::version_type ver)
  {
    a & b.majorVersion;
    a & b.minorVersion;
    a & b.timestamp;
    a & b.previousBlockHash;
    a & b.nonce;
    //------------------
    a & b.baseTransaction;
    a & b.transactionHashes;
  }
}
}

//}
