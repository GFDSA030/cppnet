#if __has_include("openssl/sha.h")
#include "D_sha.h"

#include "D_baseN.h"
#include <openssl/sha.h>

namespace fasm::hash
{
    namespace
    {
        template <typename HashFn>
        std::string hash_raw(const std::string &s, HashFn fn, size_t digest_len)
        {
            unsigned char digest[SHA512_DIGEST_LENGTH] = {};
            fn(reinterpret_cast<const unsigned char *>(s.data()), s.size(), digest);
            return std::string(reinterpret_cast<const char *>(digest), digest_len);
        }
    }

    std::string SHA1::hash(const std::string &s)
    {
        return hash_raw(s, ::SHA1, SHA_DIGEST_LENGTH);
    }

    std::string SHA1::str(const std::string &s)
    {
        return enc::base16_encode(hash(s));
    }

    std::string SHA256::hash256(const std::string &s)
    {
        return hash_raw(s, ::SHA256, SHA256_DIGEST_LENGTH);
    }

    std::string SHA256::str256(const std::string &s)
    {
        return enc::base16_encode(hash256(s));
    }

    std::string SHA256::hash224(const std::string &s)
    {
        return hash_raw(s, ::SHA224, SHA224_DIGEST_LENGTH);
    }

    std::string SHA256::str224(const std::string &s)
    {
        return enc::base16_encode(hash224(s));
    }

    std::string SHA512::hash512(const std::string &s)
    {
        return hash_raw(s, ::SHA512, SHA512_DIGEST_LENGTH);
    }

    std::string SHA512::str512(const std::string &s)
    {
        return enc::base16_encode(hash512(s));
    }

    std::string SHA512::hash384(const std::string &s)
    {
        return hash_raw(s, ::SHA384, SHA384_DIGEST_LENGTH);
    }

    std::string SHA512::str384(const std::string &s)
    {
        return enc::base16_encode(hash384(s));
    }
}
#endif
