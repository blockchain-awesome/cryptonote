#pragma once

#include "crypto.h"

namespace cryptonote
{
struct account_public_address_t
{
    crypto::public_key_t spendPublicKey;
    crypto::public_key_t viewPublicKey;
};

struct account_keys_t
{
    account_public_address_t address;
    crypto::secret_key_t spendSecretKey;
    crypto::secret_key_t viewSecretKey;
};

struct key_pair_t
{
    crypto::public_key_t publicKey;
    crypto::secret_key_t secretKey;
};
} // namespace cryptonote