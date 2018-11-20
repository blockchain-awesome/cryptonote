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
const hash_t NULL_HASH = Key::zero<hash_t>();

} // namespace cryptonote