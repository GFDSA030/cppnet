#include "D_sha.h"

#include "D_baseN.h"

#include <iomanip>
#include <sstream>

namespace fasm::hash
{
    std::vector<uint8_t> to_binary_be32(const uint32_t *h, size_t len)
    {
        std::vector<uint8_t> bin(len * 4);

        for (size_t i = 0; i < len; ++i)
        {
            uint32_t v = h[i];
            bin[i * 4 + 0] = static_cast<uint8_t>(v >> 24);
            bin[i * 4 + 1] = static_cast<uint8_t>(v >> 16);
            bin[i * 4 + 2] = static_cast<uint8_t>(v >> 8);
            bin[i * 4 + 3] = static_cast<uint8_t>(v);
        }

        return bin;
    }

    std::vector<uint8_t> to_binary_be64(const uint64_t *h, size_t len)
    {
        std::vector<uint8_t> bin(len * 8);

        for (size_t i = 0; i < len; ++i)
        {
            uint64_t v = h[i];
            bin[i * 8 + 0] = static_cast<uint8_t>(v >> 56);
            bin[i * 8 + 1] = static_cast<uint8_t>(v >> 48);
            bin[i * 8 + 2] = static_cast<uint8_t>(v >> 40);
            bin[i * 8 + 3] = static_cast<uint8_t>(v >> 32);
            bin[i * 8 + 4] = static_cast<uint8_t>(v >> 24);
            bin[i * 8 + 5] = static_cast<uint8_t>(v >> 16);
            bin[i * 8 + 6] = static_cast<uint8_t>(v >> 8);
            bin[i * 8 + 7] = static_cast<uint8_t>(v);
        }

        return bin;
    }

    /* =========================
       SHA-1
       ========================= */
    std::string SHA1::hash(const std::string &s)
    {
        SHA1 ctx;
        ctx.update(reinterpret_cast<const uint8_t *>(s.data()), s.size());
        ctx.final();
        return ctx.hex();
    }
    std::string SHA1::str(const std::string &s)
    {
        return enc::base16_encode(hash(s));
    }

    void SHA1::update(const uint8_t *d, size_t len)
    {
        buf.insert(buf.end(), d, d + len);
        bitlen += static_cast<uint64_t>(len) * 8;
        while (buf.size() >= 64)
        {
            block(buf.data());
            buf.erase(buf.begin(), buf.begin() + 64);
        }
    }

    void SHA1::final()
    {
        buf.push_back(0x80);
        while (buf.size() % 64 != 56)
            buf.push_back(0);
        // append 64-bit big-endian length
        for (int i = 7; i >= 0; --i)
            buf.push_back(static_cast<uint8_t>((bitlen >> (i * 8)) & 0xFF));
        // process all remaining 64-byte blocks (could be 1 or 2)
        for (size_t off = 0; off < buf.size(); off += 64)
            block(&buf[off]);
        buf.clear();
    }

    void SHA1::block(const uint8_t *p)
    {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i)
        {
            w[i] = (static_cast<uint32_t>(p[4 * i]) << 24) | (static_cast<uint32_t>(p[4 * i + 1]) << 16) | (static_cast<uint32_t>(p[4 * i + 2]) << 8) | (static_cast<uint32_t>(p[4 * i + 3]));
        }
        for (int i = 16; i < 80; ++i)
        {
            uint32_t t = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
            w[i] = rotl1(t);
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
        for (int i = 0; i < 80; ++i)
        {
            uint32_t f, k;
            if (i < 20)
            {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999u;
            }
            else if (i < 40)
            {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1u;
            }
            else if (i < 60)
            {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDCu;
            }
            else
            {
                f = b ^ c ^ d;
                k = 0xCA62C1D6u;
            }
            // uint32_t t = ((rotr32(a, 5)) + f + e + k + w[i]);
            // S^5(a) = left-rotate a by 5 == right-rotate a by 27
            uint32_t t = (rotr32(a, 27) + f + e + k + w[i]);
            e = d;
            d = c;
            c = rotr32(b, 2);
            b = a;
            a = t;
        }
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
    }

    std::string SHA1::hex() const
    {
        std::vector<uint8_t> hx = to_binary_be32(h, 5);
        return std::string(hx.begin(), hx.end());
    }

    /* =========================
       SHA-256 / SHA-224
       ========================= */

    std::string SHA256::hash256(const std::string &s)
    {
        return hash(s, false);
    }
    std::string SHA256::str256(const std::string &s)
    {
        return enc::base16_encode(hash256(s));
    }
    std::string SHA256::hash224(const std::string &s)
    {
        return hash(s, true);
    }
    std::string SHA256::str224(const std::string &s)
    {
        return enc::base16_encode(hash224(s));
    }
    std::string SHA256::hash(const std::string &s, bool is224)
    {
        uint32_t h[8] = {
            is224 ? 0xc1059ed8u : 0x6a09e667u,
            is224 ? 0x367cd507u : 0xbb67ae85u,
            is224 ? 0x3070dd17u : 0x3c6ef372u,
            is224 ? 0xf70e5939u : 0xa54ff53au,
            is224 ? 0xffc00b31u : 0x510e527fu,
            is224 ? 0x68581511u : 0x9b05688cu,
            is224 ? 0x64f98fa7u : 0x1f83d9abu,
            is224 ? 0xbefa4fa4u : 0x5be0cd19u};

        std::vector<uint8_t> b(s.begin(), s.end());
        uint64_t bitlen = static_cast<uint64_t>(b.size()) * 8;
        b.push_back(0x80);
        while (b.size() % 64 != 56)
            b.push_back(0);
        for (int i = 7; i >= 0; --i)
            b.push_back(static_cast<uint8_t>((bitlen >> (i * 8)) & 0xFF));

        for (size_t off = 0; off < b.size(); off += 64)
        {
            uint32_t w[64];
            for (int i = 0; i < 16; ++i)
            {
                size_t idx = off + 4 * i;
                w[i] = (static_cast<uint32_t>(b[idx]) << 24) | (static_cast<uint32_t>(b[idx + 1]) << 16) | (static_cast<uint32_t>(b[idx + 2]) << 8) | (static_cast<uint32_t>(b[idx + 3]));
            }
            for (int i = 16; i < 64; ++i)
            {
                uint32_t s1 = sigma1(w[i - 2]);
                uint32_t s0 = sigma0(w[i - 15]);
                w[i] = s1 + w[i - 7] + s0 + w[i - 16];
            }

            uint32_t a = h[0], b2 = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
            for (int i = 0; i < 64; ++i)
            {
                uint32_t T1 = hh + Sigma1(e) + ch(e, f, g) + K[i] + w[i];
                uint32_t T2 = Sigma0(a) + maj(a, b2, c);
                hh = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b2;
                b2 = a;
                a = T1 + T2;
            }
            h[0] += a;
            h[1] += b2;
            h[2] += c;
            h[3] += d;
            h[4] += e;
            h[5] += f;
            h[6] += g;
            h[7] += hh;
        }

        int words = is224 ? 7 : 8;
        std::vector<uint8_t> hx = to_binary_be32(h, words);
        return std::string(hx.begin(), hx.end());
    }
    const uint32_t SHA256::K[64] = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
        0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
        0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
        0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
        0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
        0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
        0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
        0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
        0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
        0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
        0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
        0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u};

    /* =========================
       SHA-512 / SHA-384
       ========================= */
    std::string SHA512::hash512(const std::string &s)
    {
        return hash(s, false);
    }
    std::string SHA512::str512(const std::string &s)
    {
        return enc::base16_encode(hash512(s));
    }
    std::string SHA512::hash384(const std::string &s)
    {
        return hash(s, true);
    }
    std::string SHA512::str384(const std::string &s)
    {
        return enc::base16_encode(hash384(s));
    }

    std::string SHA512::hash(const std::string &s, bool is384)
    {
        uint64_t h[8] = {
            is384 ? 0xcbbb9d5dc1059ed8ULL : 0x6a09e667f3bcc908ULL,
            is384 ? 0x629a292a367cd507ULL : 0xbb67ae8584caa73bULL,
            is384 ? 0x9159015a3070dd17ULL : 0x3c6ef372fe94f82bULL,
            is384 ? 0x152fecd8f70e5939ULL : 0xa54ff53a5f1d36f1ULL,
            is384 ? 0x67332667ffc00b31ULL : 0x510e527fade682d1ULL,
            is384 ? 0x8eb44a8768581511ULL : 0x9b05688c2b3e6c1fULL,
            is384 ? 0xdb0c2e0d64f98fa7ULL : 0x1f83d9abfb41bd6bULL,
            is384 ? 0x47b5481dbefa4fa4ULL : 0x5be0cd19137e2179ULL};

        std::vector<uint8_t> b(s.begin(), s.end());
        // compute bit length as 128-bit value: high = bytes >> 61, low = bytes * 8
        uint64_t bytes = static_cast<uint64_t>(b.size());
        uint64_t bitlen_low = bytes * 8ULL;
        uint64_t bitlen_high = (bytes >> 61); // because (bytes * 8) >> 64 == bytes >> 61

        b.push_back(0x80);
        while (b.size() % 128 != 112)
            b.push_back(0);
        // append 128-bit big-endian length
        for (int i = 15; i >= 0; --i)
        {
            uint8_t byte;
            if (i >= 8)
            {
                byte = static_cast<uint8_t>((bitlen_high >> ((i - 8) * 8)) & 0xFF);
            }
            else
            {
                byte = static_cast<uint8_t>((bitlen_low >> (i * 8)) & 0xFF);
            }
            b.push_back(byte);
        }

        for (size_t off = 0; off < b.size(); off += 128)
        {
            uint64_t w[80];
            // build w[0..15] from big-endian bytes
            for (int i = 0; i < 16; ++i)
            {
                size_t idx = off + 8 * i;
                w[i] = (static_cast<uint64_t>(b[idx]) << 56) | (static_cast<uint64_t>(b[idx + 1]) << 48) | (static_cast<uint64_t>(b[idx + 2]) << 40) | (static_cast<uint64_t>(b[idx + 3]) << 32) | (static_cast<uint64_t>(b[idx + 4]) << 24) | (static_cast<uint64_t>(b[idx + 5]) << 16) | (static_cast<uint64_t>(b[idx + 6]) << 8) | (static_cast<uint64_t>(b[idx + 7]));
            }
            for (int i = 16; i < 80; ++i)
            {
                uint64_t s1 = sigma1(w[i - 2]);
                uint64_t s0 = sigma0(w[i - 15]);
                w[i] = s1 + w[i - 7] + s0 + w[i - 16];
            }

            uint64_t a = h[0], b2 = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
            for (int i = 0; i < 80; ++i)
            {
                uint64_t T1 = hh + Sigma1(e) + ch(e, f, g) + K[i] + w[i];
                uint64_t T2 = Sigma0(a) + maj(a, b2, c);
                hh = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b2;
                b2 = a;
                a = T1 + T2;
            }
            h[0] += a;
            h[1] += b2;
            h[2] += c;
            h[3] += d;
            h[4] += e;
            h[5] += f;
            h[6] += g;
            h[7] += hh;
        }

        int words = is384 ? 6 : 8;
        std::vector<uint8_t> hx = to_binary_be64(h, words);
        return std::string(hx.begin(), hx.end());
    }

    const uint64_t SHA512::K[80] = {
        0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL,
        0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
        0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
        0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
        0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
        0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
        0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL,
        0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
        0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
        0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
        0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL,
        0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
        0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL,
        0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
        0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
        0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
        0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL,
        0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
        0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL,
        0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
        0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
        0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
        0xd192e819d6ef5218ULL, 0xd69906245565a910ULL,
        0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
        0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
        0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
        0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
        0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
        0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL,
        0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
        0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL,
        0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
        0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
        0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
        0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
        0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
        0x28db77f523047d84ULL, 0x32caab7b40c72493ULL,
        0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
        0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
        0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL};
}
