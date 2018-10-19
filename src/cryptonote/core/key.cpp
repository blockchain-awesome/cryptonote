#include "key.h"
#include "crypto/crypto.h"

namespace cryptonote
{

KeyPair Key::generate()
{
    KeyPair k;
    crypto::generate_keys(k.publicKey, k.secretKey);
    return k;
}

} // namespace cryptonote