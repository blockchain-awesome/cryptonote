#pragma once

#include "crypto.h"

namespace cryptonote
{
struct account_public_address_t
{
    public_key_t spendPublicKey;
    public_key_t viewPublicKey;
};

struct account_keys_t
{
    account_public_address_t address;
    secret_key_t spendSecretKey;
    secret_key_t viewSecretKey;
};

struct key_pair_t
{
    public_key_t publicKey;
    secret_key_t secretKey;
};
} // namespace cryptonote