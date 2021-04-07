#include "key.h"
#include "cryptonote/crypto/crypto.h"

namespace cryptonote
{

    key_pair_t Key::generate()
    {
        key_pair_t k;
        generate_keys((uint8_t *)&k.publicKey, (uint8_t *)&k.secretKey);
        return k;
    }
    const hash_t NULL_HASH = Key::zero<hash_t>();
} // namespace cryptonote