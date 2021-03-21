// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionExtra.h"

#include "stream/reader.h"
#include "stream/writer.h"
#include "common/StringTools.h"
#include "CryptoNoteTools.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include "serialization/BinaryInputStreamSerializer.h"

using namespace crypto;
using namespace Common;

namespace cryptonote
{

  bool parseTransactionExtra(const std::vector<uint8_t> &transactionExtra, std::vector<transaction_extra_t> &transactionExtraFields)
  {
    transactionExtraFields.clear();

    if (transactionExtra.empty())
      return true;

    try
    {
      // Reader iss(transactionExtra.data(), transactionExtra.size());
      const unsigned char * b = static_cast<const unsigned char *>(transactionExtra.data());
      membuf mem((char *)(b), (char *)(b + transactionExtra.size()));
      std::istream istream(&mem);
      Reader iss(istream);
      BinaryInputStreamSerializer ar(iss);

      uint8_t tag = 0;

      while (!iss.endOfStream())
      {
        iss.read(tag);
        switch (tag)
        {
        case TX_EXTRA_TAG_PADDING:
        {
          size_t size = 1;
          for (; !iss.endOfStream() && size <= TX_EXTRA_PADDING_MAX_COUNT; ++size)
          {
            uint8_t pad = 0;
            iss.read(pad);
            if (pad != 0)
            {
              return false; // all bytes should be zero
            }
          }

          if (size > TX_EXTRA_PADDING_MAX_COUNT)
          {
            return false;
          }

          transactionExtraFields.push_back(transaction_extra_padding_t{size});
          break;
        }

        case TX_EXTRA_TAG_PUBKEY:
        {
          transaction_extra_public_key_t extraPk;
          ar(extraPk.publicKey, "public_key");
          transactionExtraFields.push_back(extraPk);
          break;
        }

        case TX_EXTRA_NONCE:
        {
          transaction_extra_nonce_t extraNonce;
          uint8_t size =  0;
          iss.read(size);
          if (size > 0)
          {
            extraNonce.nonce.resize(size);
            iss.read(extraNonce.nonce.data(), extraNonce.nonce.size());
          }

          transactionExtraFields.push_back(extraNonce);
          break;
        }
        }
      }
    }
    catch (std::exception &)
    {
      return false;
    }

    return true;
  }

  struct ExtraSerializerVisitor : public boost::static_visitor<bool>
  {
    std::vector<uint8_t> &extra;

    ExtraSerializerVisitor(std::vector<uint8_t> &tx_extra)
        : extra(tx_extra) {}

    bool operator()(const transaction_extra_padding_t &t)
    {
      if (t.size > TX_EXTRA_PADDING_MAX_COUNT)
      {
        return false;
      }
      extra.insert(extra.end(), t.size, 0);
      return true;
    }

    bool operator()(const transaction_extra_public_key_t &t)
    {
      return addTransactionPublicKeyToExtra(extra, t.publicKey);
    }

    bool operator()(const transaction_extra_nonce_t &t)
    {
      return addExtraNonceToTransactionExtra(extra, t.nonce);
    }
  };

  bool writeTransactionExtra(std::vector<uint8_t> &tx_extra, const std::vector<transaction_extra_t> &tx_extra_fields)
  {
    ExtraSerializerVisitor visitor(tx_extra);

    for (const auto &tag : tx_extra_fields)
    {
      if (!boost::apply_visitor(visitor, tag))
      {
        return false;
      }
    }

    return true;
  }

  public_key_t getTransactionPublicKeyFromExtra(const std::vector<uint8_t> &tx_extra)
  {
    std::vector<transaction_extra_t> tx_extra_fields;
    parseTransactionExtra(tx_extra, tx_extra_fields);

    transaction_extra_public_key_t pub_key_field;
    if (!findTransactionExtraFieldByType(tx_extra_fields, pub_key_field))
      return boost::value_initialized<public_key_t>();

    return pub_key_field.publicKey;
  }

  bool addTransactionPublicKeyToExtra(std::vector<uint8_t> &tx_extra, const public_key_t &tx_pub_key)
  {
    tx_extra.resize(tx_extra.size() + 1 + sizeof(public_key_t));
    tx_extra[tx_extra.size() - 1 - sizeof(public_key_t)] = TX_EXTRA_TAG_PUBKEY;
    *reinterpret_cast<public_key_t *>(&tx_extra[tx_extra.size() - sizeof(public_key_t)]) = tx_pub_key;
    return true;
  }

  bool addExtraNonceToTransactionExtra(std::vector<uint8_t> &tx_extra, const binary_array_t &extra_nonce)
  {
    if (extra_nonce.size() > TX_EXTRA_NONCE_MAX_COUNT)
    {
      return false;
    }

    size_t start_pos = tx_extra.size();
    tx_extra.resize(tx_extra.size() + 2 + extra_nonce.size());
    //write tag
    tx_extra[start_pos] = TX_EXTRA_NONCE;
    //write len
    ++start_pos;
    tx_extra[start_pos] = static_cast<uint8_t>(extra_nonce.size());
    //write data
    ++start_pos;
    memcpy(&tx_extra[start_pos], extra_nonce.data(), extra_nonce.size());
    return true;
  }

  void setPaymentIdToTransactionExtraNonce(std::vector<uint8_t> &extra_nonce, const hash_t &payment_id)
  {
    extra_nonce.clear();
    extra_nonce.push_back(TX_EXTRA_NONCE_PAYMENT_ID);
    const uint8_t *payment_id_ptr = reinterpret_cast<const uint8_t *>(&payment_id);
    std::copy(payment_id_ptr, payment_id_ptr + sizeof(payment_id), std::back_inserter(extra_nonce));
  }

  bool getPaymentIdFromTransactionExtraNonce(const std::vector<uint8_t> &extra_nonce, hash_t &payment_id)
  {
    if (sizeof(hash_t) + 1 != extra_nonce.size())
      return false;
    if (TX_EXTRA_NONCE_PAYMENT_ID != extra_nonce[0])
      return false;
    payment_id = *reinterpret_cast<const hash_t *>(extra_nonce.data() + 1);
    return true;
  }

  bool parsePaymentId(const std::string &paymentIdString, hash_t &paymentId)
  {
    return hex::podFromString(paymentIdString, paymentId);
  }

  bool createTxExtraWithPaymentId(const std::string &paymentIdString, std::vector<uint8_t> &extra)
  {
    hash_t paymentIdBin;

    if (!parsePaymentId(paymentIdString, paymentIdBin))
    {
      return false;
    }

    std::vector<uint8_t> extraNonce;
    cryptonote::setPaymentIdToTransactionExtraNonce(extraNonce, paymentIdBin);

    if (!cryptonote::addExtraNonceToTransactionExtra(extra, extraNonce))
    {
      return false;
    }

    return true;
  }

  bool getPaymentIdFromTxExtra(const std::vector<uint8_t> &extra, hash_t &paymentId)
  {
    std::vector<transaction_extra_t> tx_extra_fields;
    if (!parseTransactionExtra(extra, tx_extra_fields))
    {
      return false;
    }

    transaction_extra_nonce_t extra_nonce;
    if (findTransactionExtraFieldByType(tx_extra_fields, extra_nonce))
    {
      if (!getPaymentIdFromTransactionExtraNonce(extra_nonce.nonce, paymentId))
      {
        return false;
      }
    }
    else
    {
      return false;
    }

    return true;
  }

}
