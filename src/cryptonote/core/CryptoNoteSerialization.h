// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "key.h"
#include "crypto/chacha.h"
#include "Serialization/ISerializer.h"
#include "crypto/crypto.h"

namespace crypto {

bool serialize(PublicKey& pubKey, Common::StringView name, cryptonote::ISerializer& serializer);
bool serialize(SecretKey& secKey, Common::StringView name, cryptonote::ISerializer& serializer);
bool serialize(Hash& h, Common::StringView name, cryptonote::ISerializer& serializer);
bool serialize(chacha_iv& chacha, Common::StringView name, cryptonote::ISerializer& serializer);
bool serialize(KeyImage& keyImage, Common::StringView name, cryptonote::ISerializer& serializer);
bool serialize(Signature& sig, Common::StringView name, cryptonote::ISerializer& serializer);
bool serialize(EllipticCurveScalar& ecScalar, Common::StringView name, cryptonote::ISerializer& serializer);
bool serialize(EllipticCurvePoint& ecPoint, Common::StringView name, cryptonote::ISerializer& serializer);

}

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
