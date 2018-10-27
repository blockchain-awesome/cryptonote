// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "key.h"
#include "crypto/chacha.h"
#include "serialization/ISerializer.h"
#include "crypto/crypto.h"
#include "serialize.h"

namespace cryptonote {

struct AccountKeys;

void serialize(TransactionPrefix& txP, ISerializer& serializer);
void serialize(Transaction& tx, ISerializer& serializer);
void serialize(TransactionInput& in, ISerializer& serializer);
void serialize(TransactionOutput& in, ISerializer& serializer);

void serialize(BaseInput& gen, ISerializer& serializer);
void serialize(KeyInput& key, ISerializer& serializer);
void serialize(MultisignatureInput& multisignature, ISerializer& serializer);

void serialize(TransactionOutput& output, ISerializer& serializer);
void serialize(TransactionOutputTarget& output, ISerializer& serializer);
void serialize(KeyOutput& key, ISerializer& serializer);
void serialize(MultisignatureOutput& multisignature, ISerializer& serializer);

void serialize(BlockHeader& header, ISerializer& serializer);
void serialize(Block& block, ISerializer& serializer);

void serialize(AccountPublicAddress& address, ISerializer& serializer);
void serialize(AccountKeys& keys, ISerializer& s);

void serialize(KeyPair& keyPair, ISerializer& serializer);

}
