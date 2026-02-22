#include "D_baseN.h"
#include <vector>
#include <cmath>
#include <cstring>

namespace fasm::enc
{
    uint8_t findChar(const char *d, const char s)
    {
        const char *cd = d;
        while ((*cd != s) && *cd != '\0')
            cd++;
        return cd - d;
    }
    char base64_encode_table(const int8_t d)
    {
        if (0 <= d && d < 64)
        {
            return base64_table[d];
        }
        return '=';
    }
    uint8_t base64_decode_table(const char d)
    {
        return findChar(base64_table, d);
    }

    char base32_encode_table(const int8_t d)
    {
        if (0 <= d && d < 32)
        {
            return base32_table[d];
        }
        return '=';
    }
    uint8_t base32_decode_table(const char d)
    {
        return findChar(base32_table, d);
    }

    char base16_encode_table(const int8_t d)
    {
        if (0 <= d && d < 16)
        {
            return base16_table[d];
        }
        return '=';
    }
    uint8_t base16_decode_table(const char d)
    {
        return findChar(base16_table, d);
    }

    // ------------------------------------------------------------
    // base64 (fixed, RFC4648)
    // ------------------------------------------------------------
    std::string base64_encode(const std::string &src, int32_t len)
    {
        if (len == -1)
        {
            len = src.size();
        }

        std::string out;
        out.reserve(((len + 2) / 3) * 4);

        for (int32_t i = 0; i < len; i += 3)
        {
            uint32_t v = 0;
            int n = 0;
            for (int j = 0; j < 3; ++j)
            {
                v <<= 8;
                if (i + j < len)
                {
                    v |= (uint8_t)src[i + j];
                    n++;
                }
            }

            for (int j = 0; j < 4; ++j)
            {
                if (j <= n)
                    out.push_back(base64_encode_table((v >> (18 - 6 * j)) & 0x3F));
                else
                    out.push_back('=');
            }
        }
        return out;
    }

    std::string base64_decode(const std::string &src, int32_t len)
    {
        if (len == -1)
        {
            len = src.size();
        }

        std::string out;
        out.reserve((len / 4) * 3);

        for (int32_t i = 0; i < len; i += 4)
        {
            uint32_t v = 0;
            int pad = 0;

            for (int j = 0; j < 4; ++j)
            {
                v <<= 6;
                if (src[i + j] == '=')
                    pad++;
                else
                    v |= base64_decode_table(src[i + j]);
            }

            for (int j = 0; j < 3 - pad; ++j)
                out.push_back((v >> (16 - 8 * j)) & 0xFF);
        }
        return out;
    }

    // ------------------------------------------------------------
    // base32 (fixed, RFC4648)
    // ------------------------------------------------------------
    std::string base32_encode(const std::string &src, int32_t len)
    {
        if (len == -1)
        {
            len = src.size();
        }

        std::string out;
        out.reserve(((len + 4) / 5) * 8);

        for (int32_t i = 0; i < len; i += 5)
        {
            uint64_t v = 0;
            int n = 0;

            for (int j = 0; j < 5; ++j)
            {
                v <<= 8;
                if (i + j < len)
                {
                    v |= (uint8_t)src[i + j];
                    n++;
                }
            }

            for (int j = 0; j < 8; ++j)
            {
                if (j * 5 < n * 8)
                    out.push_back(base32_encode_table((v >> (35 - 5 * j)) & 0x1F));
                else
                    out.push_back('=');
            }
        }
        return out;
    }

    std::string base32_decode(const std::string &src, int32_t len)
    {
        if (len == -1)
        {
            len = src.size();
        }

        std::string out;
        out.reserve((len / 8) * 5);

        for (int32_t i = 0; i < len; i += 8)
        {
            uint64_t v = 0;
            int pad = 0;

            for (int j = 0; j < 8; ++j)
            {
                v <<= 5;
                if (src[i + j] == '=')
                    pad++;
                else
                    v |= base32_decode_table(src[i + j]);
            }

            int bytes = (40 - pad * 5) / 8;
            for (int j = 0; j < bytes; ++j)
                out.push_back((v >> (32 - 8 * j)) & 0xFF);
        }
        return out;
    }

    // バイナリ → Base16
    std::string base16_encode(const std::string &src, int32_t len)
    {
        if (len == -1)
        {
            len = src.size();
        }

        std::string out;
        out.reserve(len * 2);

        for (int32_t i = 0; i < len; ++i)
        {
            uint8_t byte = static_cast<uint8_t>(src[i]);
            out.push_back(base16_encode_table((byte >> 4) & 0x0F));
            out.push_back(base16_encode_table(byte & 0x0F));
        }

        return out;
    }

    // Base16 → バイナリ
    std::string base16_decode(const std::string &src, int32_t len)
    {
        if (len == -1)
        {
            len = src.size();
        }

        std::string out;
        out.reserve(len / 2);

        // len は偶数であることが前提
        for (int32_t i = 0; i + 1 < len; i += 2)
        {
            uint8_t high = base16_decode_table(src[i]);
            uint8_t low = base16_decode_table(src[i + 1]);
            out.push_back(static_cast<char>((high << 4) | low));
        }

        return out;
    }
}
#if __has_include("boost/multiprecision/cpp_int.hpp")
#include <boost/multiprecision/cpp_int.hpp>
namespace fasm::enc
{
    using boost::multiprecision::cpp_int;

    uint8_t baseN_decode_table(const char d, const char *table)
    {
        return findChar(table, d);
    }
    char baseN_encode_table(const size_t d, const char *table)
    {
        if (0 <= d && d < strlen(table))
        {
            return table[d];
        }
        return '=';
    }
    // ------------------------------------------------------------
    // generic baseN (fixed length, cpp_int)
    // ------------------------------------------------------------
    static int32_t baseN_fixed_len(size_t in_len, const char *table)
    {
        double bits = in_len * 8.0;
        return (size_t)std::ceil(bits / std::log2(strlen(table)));
    }

    std::string baseN_encode(const std::string &src, int32_t len, const char *table)
    {
        if (len == -1)
        {
            len = src.size();
        }

        const size_t base = std::strlen(table);
        const size_t out_len = baseN_fixed_len(len, table);

        cpp_int num = 0;
        for (int32_t i = 0; i < len; ++i)
        {
            num <<= 8;
            num |= (uint8_t)src[i];
        }

        std::string out(out_len, table[0]);
        for (size_t i = 0; i < out_len; ++i)
        {
            cpp_int rem = num % base;
            num /= base;
            out[out_len - 1 - i] = baseN_encode_table((size_t)rem, table);
        }
        return out;
    }

    std::string baseN_decode(const std::string &src, int32_t len, const char *table)
    {
        if (len == -1)
        {
            len = src.size();
        }

        const size_t base = std::strlen(table);

        cpp_int num = 0;
        for (int32_t i = 0; i < len; ++i)
        {
            num *= base;
            num += baseN_decode_table(src[i], table);
        }

        std::vector<uint8_t> bytes;
        while (num > 0)
        {
            bytes.push_back((uint8_t)(num & 0xFF));
            num >>= 8;
        }

        std::string out(bytes.rbegin(), bytes.rend());
        return out;
    }

    std::string baseN_encode(const std::string &src, const char *table, int32_t len)
    {
        return baseN_encode(src, len, table);
    }
    std::string baseN_decode(const std::string &src, const char *table, int32_t len)
    {
        return baseN_decode(src, len, table);
    }
}
#endif
