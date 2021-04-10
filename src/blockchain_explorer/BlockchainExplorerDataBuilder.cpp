// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BlockchainExplorerDataBuilder.h"

#include <boost/utility/value_init.hpp>
#include <boost/range/combine.hpp>
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/structures/block_entry.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/transaction/TransactionExtra.h"
#include "CryptoNoteConfig.h"
#include "stream/block.h"
#include "stream/transaction.h"
#include "cryptonote/structures/array.hpp"

namespace cryptonote {

BlockchainExplorerDataBuilder::BlockchainExplorerDataBuilder(cryptonote::ICore& core, const cryptonote::ICryptoNoteProtocolQuery& protocol) :
core(core),
protocol(protocol) {
}

bool BlockchainExplorerDataBuilder::getMixin(const transaction_t& transaction, uint64_t& mixin) {
  mixin = 0;
  for (const transaction_input_t& txin : transaction.inputs) {
    if (txin.type() != typeid(key_input_t)) {
      continue;
    }
    uint64_t currentMixin = boost::get<key_input_t>(txin).outputIndexes.size();
    if (currentMixin > mixin) {
      mixin = currentMixin;
    }
  }
  return true;
}

bool BlockchainExplorerDataBuilder::getPaymentId(const transaction_t& transaction, hash_t& paymentId) {
  std::vector<transaction_extra_t> txExtraFields;
  parseTransactionExtra(transaction.extra, txExtraFields);
  transaction_extra_nonce_t extraNonce;
  if (!findTransactionExtraFieldByType(txExtraFields, extraNonce)) {
    return false;
  }
  return getPaymentIdFromTransactionExtraNonce(extraNonce.nonce, paymentId);
}

bool BlockchainExplorerDataBuilder::fillTxExtra(const std::vector<uint8_t>& rawExtra, transaction_extra_details_t& extraDetails) {
  extraDetails.raw = rawExtra;
  std::vector<transaction_extra_t> txExtraFields;
  parseTransactionExtra(rawExtra, txExtraFields);
  for (const transaction_extra_t& field : txExtraFields) {
    if (typeid(transaction_extra_padding_t) == field.type()) {
      extraDetails.padding.push_back(std::move(boost::get<transaction_extra_padding_t>(field).size));
    } else if (typeid(transaction_extra_public_key_t) == field.type()) {
      extraDetails.publicKey.push_back(std::move(boost::get<transaction_extra_public_key_t>(field).publicKey));
    } else if (typeid(transaction_extra_nonce_t) == field.type()) {
      extraDetails.nonce.push_back(hex::to(boost::get<transaction_extra_nonce_t>(field).nonce.data(), boost::get<transaction_extra_nonce_t>(field).nonce.size()));
    }
  }
  return true;
}

size_t BlockchainExplorerDataBuilder::median(std::vector<size_t>& v) {
  if (v.empty())
    return boost::value_initialized<size_t>();
  if (v.size() == 1)
    return v[0];

  size_t n = (v.size()) / 2;
  std::sort(v.begin(), v.end());
  //nth_element(v.begin(), v.begin()+n-1, v.end());
  if (v.size() % 2) {//1, 3, 5...
    return v[n];
  } else {//2, 4, 6...
    return (v[n - 1] + v[n]) / 2;
  }

}

bool BlockchainExplorerDataBuilder::fillBlockDetails(const block_t&block, block_details_t& blockDetails) {
  hash_t hash = Block::getHash(block);

  blockDetails.majorVersion = block.majorVersion;
  blockDetails.minorVersion = block.minorVersion;
  blockDetails.timestamp = block.timestamp;
  blockDetails.prevBlockHash = block.previousBlockHash;
  blockDetails.nonce = block.nonce;
  blockDetails.hash = hash;

  blockDetails.reward = 0;
  for (const transaction_output_t& out : block.baseTransaction.outputs) {
    blockDetails.reward += out.amount;
  }

  if (block.baseTransaction.inputs.front().type() != typeid(base_input_t))
    return false;
  blockDetails.height = boost::get<base_input_t>(block.baseTransaction.inputs.front()).blockIndex;

  hash_t tmpHash = core.getBlockIdByHeight(blockDetails.height);
  blockDetails.isOrphaned = hash != tmpHash;

  if (!core.getBlockDifficulty(blockDetails.height, blockDetails.difficulty)) {
    return false;
  }

  std::vector<size_t> blocksSizes;
  if (!core.getBackwardBlocksSizes(blockDetails.height, blocksSizes, parameters::CRYPTONOTE_REWARD_BLOCKS_WINDOW)) {
    return false;
  }
  blockDetails.sizeMedian = median(blocksSizes);

  size_t blockSize = 0;
  if (!core.getBlockSize(hash, blockSize)) {
    return false;
  }
  blockDetails.transactionsCumulativeSize = blockSize;

  size_t blokBlobSize = BinaryArray::size(block);
  size_t minerTxBlobSize = BinaryArray::size(block.baseTransaction);
  blockDetails.blockSize = blokBlobSize + blockDetails.transactionsCumulativeSize - minerTxBlobSize;

  if (!core.getAlreadyGeneratedCoins(hash, blockDetails.alreadyGeneratedCoins)) {
    return false;
  }

  if (!core.getGeneratedTransactionsNumber(blockDetails.height, blockDetails.alreadyGeneratedTransactions)) {
    return false;
  }

  uint64_t prevBlockGeneratedCoins = 0;
  if (blockDetails.height > 0) {
    if (!core.getAlreadyGeneratedCoins(block.previousBlockHash, prevBlockGeneratedCoins)) {
      return false;
    }
  }
  uint64_t maxReward = 0;
  uint64_t currentReward = 0;
  int64_t emissionChange = 0;
  if (!core.getBlockReward(blockDetails.sizeMedian, 0, prevBlockGeneratedCoins, 0, maxReward, emissionChange)) {
    return false;
  }
  if (!core.getBlockReward(blockDetails.sizeMedian, blockDetails.transactionsCumulativeSize, prevBlockGeneratedCoins, 0, currentReward, emissionChange)) {
    return false;
  }

  blockDetails.baseReward = maxReward;
  if (maxReward == 0 && currentReward == 0) {
    blockDetails.penalty = static_cast<double>(0);
  } else {
    if (maxReward < currentReward) {
      return false;
    }
    blockDetails.penalty = static_cast<double>(maxReward - currentReward) / static_cast<double>(maxReward);
  }


  blockDetails.transactions.reserve(block.transactionHashes.size() + 1);
  transaction_explorer_details_t transactionDetails;
  if (!fillTransactionDetails(block.baseTransaction, transactionDetails, block.timestamp)) {
    return false;
  }
  blockDetails.transactions.push_back(std::move(transactionDetails));

  std::list<transaction_t> found;
  std::list<hash_t> missed;
  core.getTransactions(block.transactionHashes, found, missed, blockDetails.isOrphaned);
  if (found.size() != block.transactionHashes.size()) {
    return false;
  }

  blockDetails.totalFeeAmount = 0;

  for (const transaction_t& tx : found) {
    transaction_explorer_details_t transactionDetails;
    if (!fillTransactionDetails(tx, transactionDetails, block.timestamp)) {
      return false;
    }
    blockDetails.transactions.push_back(std::move(transactionDetails));
    blockDetails.totalFeeAmount += transactionDetails.fee;
  }
  return true;
}

bool BlockchainExplorerDataBuilder::fillTransactionDetails(const transaction_t& transaction, transaction_explorer_details_t& transactionDetails, uint64_t timestamp) {
  hash_t hash = BinaryArray::objectHash(transaction);
  transactionDetails.hash = hash;

  transactionDetails.timestamp = timestamp;

  hash_t blockHash;
  uint32_t blockHeight;
  if (!core.getBlockContainingTx(hash, blockHash, blockHeight)) {
    transactionDetails.inBlockchain = false;
    transactionDetails.blockHeight = boost::value_initialized<uint32_t>();
    transactionDetails.blockHash = boost::value_initialized<hash_t>();
  } else {
    transactionDetails.inBlockchain = true;
    transactionDetails.blockHeight = blockHeight;
    transactionDetails.blockHash = blockHash;
    if (timestamp == 0) {
      block_t block;
      if (!core.getBlockByHash(blockHash, block)) {
        return false;
      }
      transactionDetails.timestamp = block.timestamp;
    }
  }

  transactionDetails.size = BinaryArray::size(transaction);
  transactionDetails.unlockTime = transaction.unlockTime;
  transactionDetails.totalOutputsAmount = get_outs_money_amount(transaction);

  uint64_t inputsAmount;
  if (!get_inputs_money_amount(transaction, inputsAmount)) {
    return false;
  }
  transactionDetails.totalInputsAmount = inputsAmount;

  if (transaction.inputs.size() > 0 && transaction.inputs.front().type() == typeid(base_input_t)) {
    //It's gen transaction
    transactionDetails.fee = 0;
    transactionDetails.mixin = 0;
  } else {
    uint64_t fee;
    if (!get_tx_fee(transaction, fee)) {
      return false;
    }
    transactionDetails.fee = fee;
    uint64_t mixin;
    if (!getMixin(transaction, mixin)) {
      return false;
    }
    transactionDetails.mixin = mixin;
  }

  hash_t paymentId;
  if (getPaymentId(transaction, paymentId)) {
    transactionDetails.paymentId = paymentId;
  } else {
    transactionDetails.paymentId = boost::value_initialized<hash_t>();
  }

  fillTxExtra(transaction.extra, transactionDetails.extra);

  transactionDetails.signatures.reserve(transaction.signatures.size());
  for (const std::vector<signature_t>& signatures : transaction.signatures) {
    std::vector<signature_t> signaturesDetails;
    signaturesDetails.reserve(signatures.size());
    for (const signature_t& signature : signatures) {
      signaturesDetails.push_back(std::move(signature));
    }
    transactionDetails.signatures.push_back(std::move(signaturesDetails));
  }

  transactionDetails.inputs.reserve(transaction.inputs.size());
  for (const transaction_input_t& txIn : transaction.inputs) {
    transaction_input_details_t txInDetails;

    if (txIn.type() == typeid(base_input_t)) {
      transaction_input_generate_details_t txInGenDetails;
      txInGenDetails.height = boost::get<base_input_t>(txIn).blockIndex;
      txInDetails.amount = 0;
      for (const transaction_output_t& out : transaction.outputs) {
        txInDetails.amount += out.amount;
      }
      txInDetails.input = txInGenDetails;
    } else if (txIn.type() == typeid(key_input_t)) {
      transaction_input_to_key_details_t txInToKeyDetails;
      const key_input_t& txInToKey = boost::get<key_input_t>(txIn);
      std::list<std::pair<hash_t, size_t>> outputReferences;
      if (!core.scanOutputkeysForIndices(txInToKey, outputReferences)) {
        return false;
      }
      txInDetails.amount = txInToKey.amount;
      txInToKeyDetails.outputIndexes = txInToKey.outputIndexes;
      txInToKeyDetails.keyImage = txInToKey.keyImage;
      txInToKeyDetails.mixin = txInToKey.outputIndexes.size();
      txInToKeyDetails.output.number = outputReferences.back().second;
      txInToKeyDetails.output.transactionHash = outputReferences.back().first;
      txInDetails.input = txInToKeyDetails;
    } else if (txIn.type() == typeid(multi_signature_input_t)) {
      transaction_input_multisignature_details_t txInMultisigDetails;
      const multi_signature_input_t& txInMultisig = boost::get<multi_signature_input_t>(txIn);
      txInDetails.amount = txInMultisig.amount;
      txInMultisigDetails.signatures = txInMultisig.signatureCount;
      std::pair<hash_t, size_t> outputReference;
      if (!core.getMultisigOutputReference(txInMultisig, outputReference)) {
        return false;
      }
      txInMultisigDetails.output.number = outputReference.second;
      txInMultisigDetails.output.transactionHash = outputReference.first;
      txInDetails.input = txInMultisigDetails;
    } else {
      return false;
    }
    transactionDetails.inputs.push_back(std::move(txInDetails));
  }

  transactionDetails.outputs.reserve(transaction.outputs.size());
  std::vector<uint32_t> globalIndices;
  globalIndices.reserve(transaction.outputs.size());
  if (!transactionDetails.inBlockchain || !core.get_tx_outputs_gindexs(hash, globalIndices)) {
    for (size_t i = 0; i < transaction.outputs.size(); ++i) {
      globalIndices.push_back(0);
    }
  }

  typedef boost::tuple<transaction_output_t, uint32_t> outputWithIndex;
  auto range = boost::combine(transaction.outputs, globalIndices);
  for (const outputWithIndex& txOutput : range) {
    transaction_output_details_t txOutDetails;
    txOutDetails.amount = txOutput.get<0>().amount;
    txOutDetails.globalIndex = txOutput.get<1>();

    if (txOutput.get<0>().target.type() == typeid(key_output_t)) {
      transaction_output_to_key_details_t txOutToKeyDetails;
      txOutToKeyDetails.txOutKey = boost::get<key_output_t>(txOutput.get<0>().target).key;
      txOutDetails.output = txOutToKeyDetails;
    } else if (txOutput.get<0>().target.type() == typeid(multi_signature_output_t)) {
      transaction_output_multi_signature_details_t txOutMultisigDetails;
      multi_signature_output_t txOutMultisig = boost::get<multi_signature_output_t>(txOutput.get<0>().target);
      txOutMultisigDetails.keys.reserve(txOutMultisig.keys.size());
      for (const public_key_t& key : txOutMultisig.keys) {
        txOutMultisigDetails.keys.push_back(std::move(key));
      }
      txOutMultisigDetails.requiredSignatures = txOutMultisig.requiredSignatureCount;
      txOutDetails.output = txOutMultisigDetails;
    } else {
      return false;
    }
    transactionDetails.outputs.push_back(std::move(txOutDetails));
  }

  return true;
}

}
