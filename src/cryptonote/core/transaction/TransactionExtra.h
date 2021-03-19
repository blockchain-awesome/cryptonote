// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include <boost/variant.hpp>

#include <cryptonote.h>

#define TX_EXTRA_PADDING_MAX_COUNT          255
#define TX_EXTRA_NONCE_MAX_COUNT            255

#define TX_EXTRA_TAG_PADDING                0x00
#define TX_EXTRA_TAG_PUBKEY                 0x01
#define TX_EXTRA_NONCE                      0x02

#define TX_EXTRA_NONCE_PAYMENT_ID           0x00

namespace cryptonote {

struct transaction_extra_padding_t {
  size_t size;
};

struct transaction_extra_public_key_t {
  public_key_t publicKey;
};

struct transaction_extra_nonce_t {
  std::vector<uint8_t> nonce;
};

// tx_extra_field format, except tx_extra_padding and tx_extra_pub_key:
//   varint tag;
//   varint size;
//   varint data[];
typedef boost::variant<transaction_extra_padding_t, transaction_extra_public_key_t, transaction_extra_nonce_t> transaction_extra_field_t;



template<typename T>
bool findTransactionExtraFieldByType(const std::vector<transaction_extra_field_t>& tx_extra_fields, T& field) {
  auto it = std::find_if(tx_extra_fields.begin(), tx_extra_fields.end(),
    [](const transaction_extra_field_t& f) { return typeid(T) == f.type(); });

  if (tx_extra_fields.end() == it)
    return false;

  field = boost::get<T>(*it);
  return true;
}

bool parseTransactionExtra(const std::vector<uint8_t>& tx_extra, std::vector<transaction_extra_field_t>& tx_extra_fields);
bool writeTransactionExtra(std::vector<uint8_t>& tx_extra, const std::vector<transaction_extra_field_t>& tx_extra_fields);

public_key_t getTransactionPublicKeyFromExtra(const std::vector<uint8_t>& tx_extra);
bool addTransactionPublicKeyToExtra(std::vector<uint8_t>& tx_extra, const public_key_t& tx_pub_key);
bool addExtraNonceToTransactionExtra(std::vector<uint8_t>& tx_extra, const binary_array_t& extra_nonce);
void setPaymentIdToTransactionExtraNonce(binary_array_t& extra_nonce, const hash_t& payment_id);
bool getPaymentIdFromTransactionExtraNonce(const binary_array_t& extra_nonce, hash_t& payment_id);

bool createTxExtraWithPaymentId(const std::string& paymentIdString, std::vector<uint8_t>& extra);
//returns false if payment id is not found or parse error
bool getPaymentIdFromTxExtra(const std::vector<uint8_t>& extra, hash_t& paymentId);
bool parsePaymentId(const std::string& paymentIdString, hash_t& paymentId);

}
