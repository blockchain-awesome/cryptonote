#pragma once

#include "crypto.h"

namespace cryptonote
{
struct AccountPublicAddress
{
    crypto::PublicKey spendPublicKey;
    crypto::PublicKey viewPublicKey;
};

struct AccountKeys
{
    AccountPublicAddress address;
    crypto::SecretKey spendSecretKey;
    crypto::SecretKey viewSecretKey;
};

struct KeyPair
{
    crypto::PublicKey publicKey;
    crypto::SecretKey secretKey;
};
} // namespace cryptonote