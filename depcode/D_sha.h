#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
namespace fasm::inline hash
{
    static inline uint32_t rotr32(uint32_t x, unsigned n)
    {
        return (x >> n) | (x << (32 - n));
    }
    static inline uint64_t rotr64(uint64_t x, unsigned n)
    {
        return (x >> n) | (x << (64 - n));
    }

    /* =========================
       SHA-1
       ========================= */
    class SHA1
    {
    public:
        static std::string hash(const std::string &s);
        static std::string str(const std::string &s);

    private:
        uint32_t h[5] = {
            0x67452301u, 0xEFCDAB89u, 0x98BADCFEu,
            0x10325476u, 0xC3D2E1F0u};
        std::vector<uint8_t> buf;
        uint64_t bitlen = 0;

        void update(const uint8_t *d, size_t len);

        void final();

        static inline uint32_t rotl1(uint32_t x) { return (x << 1) | (x >> 31); }

        void block(const uint8_t *p);
        // std::string to_hex() const;
        std::string hex() const;
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

    private:
        static const uint32_t K[64];

        static inline uint32_t sigma0(uint32_t x) { return rotr32(x, 7) ^ rotr32(x, 18) ^ (x >> 3); }
        static inline uint32_t sigma1(uint32_t x) { return rotr32(x, 17) ^ rotr32(x, 19) ^ (x >> 10); }
        static inline uint32_t Sigma0(uint32_t x) { return rotr32(x, 2) ^ rotr32(x, 13) ^ rotr32(x, 22); }
        static inline uint32_t Sigma1(uint32_t x) { return rotr32(x, 6) ^ rotr32(x, 11) ^ rotr32(x, 25); }
        static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ ((~x) & z); }
        static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }

        static std::string hash(const std::string &s, bool is224);
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

    private:
        static const uint64_t K[80];

        static inline uint64_t Sigma0(uint64_t x) { return rotr64(x, 28) ^ rotr64(x, 34) ^ rotr64(x, 39); }
        static inline uint64_t Sigma1(uint64_t x) { return rotr64(x, 14) ^ rotr64(x, 18) ^ rotr64(x, 41); }
        static inline uint64_t sigma0(uint64_t x) { return rotr64(x, 1) ^ rotr64(x, 8) ^ (x >> 7); }
        static inline uint64_t sigma1(uint64_t x) { return rotr64(x, 19) ^ rotr64(x, 61) ^ (x >> 6); }
        static inline uint64_t ch(uint64_t x, uint64_t y, uint64_t z) { return (x & y) ^ ((~x) & z); }
        static inline uint64_t maj(uint64_t x, uint64_t y, uint64_t z) { return (x & y) ^ (x & z) ^ (y & z); }

        static std::string hash(const std::string &s, bool is384);
    };
}
