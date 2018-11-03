#pragma once

#include "crypto.h"

namespace cryptonote
{
struct AccountPublicAddress
{
    crypto::public_key_t spendPublicKey;
    crypto::public_key_t viewPublicKey;
};

struct AccountKeys
{
    AccountPublicAddress address;
    crypto::secret_key_t spendSecretKey;
    crypto::secret_key_t viewSecretKey;
};

struct KeyPair
{
    crypto::public_key_t publicKey;
    crypto::secret_key_t secretKey;
};
} // namespace cryptonote