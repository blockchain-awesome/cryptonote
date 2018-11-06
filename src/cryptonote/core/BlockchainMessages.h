// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>

#include <cryptonote.h>

namespace cryptonote {

class NewBlockMessage {
public:
  NewBlockMessage(const crypto::hash_t& hash);
  NewBlockMessage() = default;
  void get(crypto::hash_t& hash) const;
private:
  crypto::hash_t blockHash;
};

class NewAlternativeBlockMessage {
public:
  NewAlternativeBlockMessage(const crypto::hash_t& hash);
  NewAlternativeBlockMessage() = default;
  void get(crypto::hash_t& hash) const;
private:
  crypto::hash_t blockHash;
};

class ChainSwitchMessage {
public:
  ChainSwitchMessage(std::vector<crypto::hash_t>&& hashes);
  ChainSwitchMessage(const ChainSwitchMessage& other);
  void get(std::vector<crypto::hash_t>& hashes) const;
private:
  std::vector<crypto::hash_t> blocksFromCommonRoot;
};

class BlockchainMessage {
public:
  enum class MessageType {
    NEW_BLOCK_MESSAGE,
    NEW_ALTERNATIVE_BLOCK_MESSAGE,
    CHAIN_SWITCH_MESSAGE
  };

  BlockchainMessage(NewBlockMessage&& message);
  BlockchainMessage(NewAlternativeBlockMessage&& message);
  BlockchainMessage(ChainSwitchMessage&& message);

  BlockchainMessage(const BlockchainMessage& other);

  ~BlockchainMessage();

  MessageType getType() const;

  bool getNewBlockHash(crypto::hash_t& hash) const;
  bool getNewAlternativeBlockHash(crypto::hash_t& hash) const;
  bool getChainSwitch(std::vector<crypto::hash_t>& hashes) const;
private:
  const MessageType type;

  union {
    NewBlockMessage newBlockMessage;
    NewAlternativeBlockMessage newAlternativeBlockMessage;
    ChainSwitchMessage* chainSwitchMessage;
  };
};

}
