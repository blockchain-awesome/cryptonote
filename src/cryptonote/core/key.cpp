#include "key.h"
#include "crypto/crypto.h"

namespace cryptonote
{

key_pair_t Key::generate()
{
    key_pair_t k;
    crypto::generate_keys(k.publicKey, k.secretKey);
    return k;
}

} // namespace cryptonote