#pragma once
#if __has_include("openssl/sha.h")
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
namespace fasm::hash
{
    /* =========================
       SHA-1
       ========================= */
    class SHA1
    {
    public:
        static std::string hash(const std::string &s);
        static std::string str(const std::string &s);
    };

    /* =========================
       SHA-256 / SHA-224
       ========================= */

    class SHA256
    {
    public:
        static std::string hash256(const std::string &s);
        static std::string str256(const std::string &s);
        static std::string hash224(const std::string &s);
        static std::string str224(const std::string &s);
    };

    /* =========================
       SHA-512 / SHA-384
       ========================= */

    class SHA512
    {
    public:
        static std::string hash512(const std::string &s);
        static std::string str512(const std::string &s);
        static std::string hash384(const std::string &s);
        static std::string str384(const std::string &s);
    };
}
#endif
