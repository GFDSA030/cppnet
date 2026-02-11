#include "aes.h"
#include <random>
#include <stdexcept>
#include <cstring>

namespace cryptASM
{

    AES128::AES128()
    {
        key.fill(0);
        expand_key();
    }

    AES128::AES128(const std::vector<uint8_t> &key128)
    {
        if (key128.size() != 16)
            throw std::runtime_error("AES-128 requires 16-byte key");

        std::copy(key128.begin(), key128.end(), key.begin());
        expand_key();
    }

    std::vector<uint8_t> AES128::encrypt(const std::vector<uint8_t> &plain) const
    {
        std::vector<uint8_t> data = pad(plain);
        auto iv = random_block();

        std::vector<uint8_t> out(iv.begin(), iv.end());
        std::array<uint8_t, BLOCK_SIZE> prev = iv;

        for (size_t i = 0; i < data.size(); i += BLOCK_SIZE)
        {
            std::array<uint8_t, BLOCK_SIZE> block;
            std::memcpy(block.data(), &data[i], BLOCK_SIZE);

            for (size_t j = 0; j < BLOCK_SIZE; ++j)
                block[j] ^= prev[j];

            encrypt_block(block);
            out.insert(out.end(), block.begin(), block.end());
            prev = block;
        }
        return out;
    }

    std::vector<uint8_t> AES128::decrypt(const std::vector<uint8_t> &cipher) const
    {
        if (cipher.size() < BLOCK_SIZE || cipher.size() % BLOCK_SIZE != 0)
            throw std::runtime_error("Invalid AES-CBC ciphertext");

        std::array<uint8_t, BLOCK_SIZE> iv;
        std::memcpy(iv.data(), cipher.data(), BLOCK_SIZE);

        std::vector<uint8_t> out;
        std::array<uint8_t, BLOCK_SIZE> prev = iv;

        for (size_t i = BLOCK_SIZE; i < cipher.size(); i += BLOCK_SIZE)
        {
            std::array<uint8_t, BLOCK_SIZE> block;
            std::memcpy(block.data(), &cipher[i], BLOCK_SIZE);

            auto tmp = block;
            decrypt_block(block);

            for (size_t j = 0; j < BLOCK_SIZE; ++j)
                block[j] ^= prev[j];

            out.insert(out.end(), block.begin(), block.end());
            prev = tmp;
        }
        return unpad(out);
    }

    // ================= AES Core =================

    const uint8_t AES128::sbox[256] =
        {
            0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
            0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
            0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
            0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
            0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
            0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
            0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
            0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
            0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
            0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
            0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
            0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
            0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
            0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
            0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
            0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16};

    const uint8_t AES128::rsbox[256] =
        {
            0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
            0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
            0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
            0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
            0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
            0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
            0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
            0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
            0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
            0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
            0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
            0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
            0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
            0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
            0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
            0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D};

    // ================= 内部処理 =================

    uint8_t AES128::xtime(uint8_t x)
    {
        return (x << 1) ^ ((x & 0x80) ? 0x1B : 0);
    }

    uint8_t AES128::mul(uint8_t a, uint8_t b)
    {
        uint8_t r = 0;
        while (b)
        {
            if (b & 1)
                r ^= a;
            a = xtime(a);
            b >>= 1;
        }
        return r;
    }

    void AES128::expand_key()
    {
        static constexpr uint8_t rcon[10] =
            {
                0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};

        std::memcpy(round_key.data(), key.data(), 16);

        uint8_t tmp[4];
        int bytes = 16, r = 0;

        while (bytes < 176)
        {
            std::memcpy(tmp, &round_key[bytes - 4], 4);

            if (bytes % 16 == 0)
            {
                uint8_t t = tmp[0];
                tmp[0] = sbox[tmp[1]] ^ rcon[r++];
                tmp[1] = sbox[tmp[2]];
                tmp[2] = sbox[tmp[3]];
                tmp[3] = sbox[t];
            }

            for (int i = 0; i < 4; ++i)
            {
                round_key[bytes] = round_key[bytes - 16] ^ tmp[i];
                bytes++;
            }
        }
    }

    void AES128::add_round_key(uint8_t *state, int round) const
    {
        for (int i = 0; i < 16; ++i)
            state[i] ^= round_key[round * 16 + i];
    }

    void AES128::encrypt_block(std::array<uint8_t, 16> &s) const
    {
        add_round_key(s.data(), 0);

        for (int r = 1; r < 10; ++r)
        {
            sub_bytes(s);
            shift_rows(s);
            mix_columns(s);
            add_round_key(s.data(), r);
        }

        sub_bytes(s);
        shift_rows(s);
        add_round_key(s.data(), 10);
    }

    void AES128::decrypt_block(std::array<uint8_t, 16> &s) const
    {
        add_round_key(s.data(), 10);

        for (int r = 9; r > 0; --r)
        {
            inv_shift_rows(s);
            inv_sub_bytes(s);
            add_round_key(s.data(), r);
            inv_mix_columns(s);
        }

        inv_shift_rows(s);
        inv_sub_bytes(s);
        add_round_key(s.data(), 0);
    }

    void AES128::sub_bytes(std::array<uint8_t, 16> &s)
    {
        for (auto &b : s)
            b = sbox[b];
    }

    void AES128::inv_sub_bytes(std::array<uint8_t, 16> &s)
    {
        for (auto &b : s)
            b = rsbox[b];
    }

    void AES128::shift_rows(std::array<uint8_t, 16> &s)
    {
        std::array<uint8_t, 16> t = s;
        s[1] = t[5];
        s[5] = t[9];
        s[9] = t[13];
        s[13] = t[1];
        s[2] = t[10];
        s[6] = t[14];
        s[10] = t[2];
        s[14] = t[6];
        s[3] = t[15];
        s[7] = t[3];
        s[11] = t[7];
        s[15] = t[11];
    }

    void AES128::inv_shift_rows(std::array<uint8_t, 16> &s)
    {
        std::array<uint8_t, 16> t = s;
        s[1] = t[13];
        s[5] = t[1];
        s[9] = t[5];
        s[13] = t[9];
        s[2] = t[10];
        s[6] = t[14];
        s[10] = t[2];
        s[14] = t[6];
        s[3] = t[7];
        s[7] = t[11];
        s[11] = t[15];
        s[15] = t[3];
    }

    void AES128::mix_columns(std::array<uint8_t, 16> &s)
    {
        for (int i = 0; i < 4; ++i)
        {
            uint8_t *c = &s[i * 4];
            uint8_t a = c[0], b = c[1], d = c[2], e = c[3];
            c[0] = mul(a, 2) ^ mul(b, 3) ^ d ^ e;
            c[1] = a ^ mul(b, 2) ^ mul(d, 3) ^ e;
            c[2] = a ^ b ^ mul(d, 2) ^ mul(e, 3);
            c[3] = mul(a, 3) ^ b ^ d ^ mul(e, 2);
        }
    }

    void AES128::inv_mix_columns(std::array<uint8_t, 16> &s)
    {
        for (int i = 0; i < 4; ++i)
        {
            uint8_t *c = &s[i * 4];
            uint8_t a = c[0], b = c[1], d = c[2], e = c[3];
            c[0] = mul(a, 14) ^ mul(b, 11) ^ mul(d, 13) ^ mul(e, 9);
            c[1] = mul(a, 9) ^ mul(b, 14) ^ mul(d, 11) ^ mul(e, 13);
            c[2] = mul(a, 13) ^ mul(b, 9) ^ mul(d, 14) ^ mul(e, 11);
            c[3] = mul(a, 11) ^ mul(b, 13) ^ mul(d, 9) ^ mul(e, 14);
        }
    }

    std::vector<uint8_t> AES128::pad(const std::vector<uint8_t> &in)
    {
        size_t p = BLOCK_SIZE - (in.size() % BLOCK_SIZE);
        std::vector<uint8_t> out = in;
        out.insert(out.end(), p, static_cast<uint8_t>(p));
        return out;
    }

    std::vector<uint8_t> AES128::unpad(const std::vector<uint8_t> &in)
    {
        uint8_t p = in.back();
        if (p == 0 || p > BLOCK_SIZE)
            throw std::runtime_error("Invalid padding");
        return {in.begin(), in.end() - p};
    }

    std::array<uint8_t, 16> AES128::random_block()
    {
        std::array<uint8_t, 16> b{};
        std::random_device rd;
        for (auto &v : b)
            v = static_cast<uint8_t>(rd());
        return b;
    }

    AES256::AES256()
    {
        key.fill(0);
        expand_key();
    }

    AES256::AES256(const std::vector<uint8_t> &key256)
    {
        if (key256.size() != KEY_SIZE)
            throw std::runtime_error("AES-256 requires 32-byte key");

        std::copy(key256.begin(), key256.end(), key.begin());
        expand_key();
    }

    std::vector<uint8_t> AES256::encrypt(const std::vector<uint8_t> &plain) const
    {
        auto data = pad(plain);
        auto iv = random_block();

        std::vector<uint8_t> out(iv.begin(), iv.end());
        std::array<uint8_t, BLOCK_SIZE> prev = iv;

        for (size_t i = 0; i < data.size(); i += BLOCK_SIZE)
        {
            std::array<uint8_t, BLOCK_SIZE> block;
            std::memcpy(block.data(), &data[i], BLOCK_SIZE);

            for (int j = 0; j < 16; ++j)
                block[j] ^= prev[j];

            encrypt_block(block);
            out.insert(out.end(), block.begin(), block.end());
            prev = block;
        }
        return out;
    }

    std::vector<uint8_t> AES256::decrypt(const std::vector<uint8_t> &cipher) const
    {
        if (cipher.size() < BLOCK_SIZE || cipher.size() % BLOCK_SIZE != 0)
            throw std::runtime_error("Invalid AES-CBC ciphertext");

        std::array<uint8_t, BLOCK_SIZE> iv;
        std::memcpy(iv.data(), cipher.data(), BLOCK_SIZE);

        std::vector<uint8_t> out;
        std::array<uint8_t, BLOCK_SIZE> prev = iv;

        for (size_t i = BLOCK_SIZE; i < cipher.size(); i += BLOCK_SIZE)
        {
            std::array<uint8_t, BLOCK_SIZE> block;
            std::memcpy(block.data(), &cipher[i], BLOCK_SIZE);

            auto tmp = block;
            decrypt_block(block);

            for (int j = 0; j < 16; ++j)
                block[j] ^= prev[j];

            out.insert(out.end(), block.begin(), block.end());
            prev = tmp;
        }
        return unpad(out);
    }

    // ================== AES Tables ==================

    const uint8_t AES256::sbox[256] = {
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
        0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
        0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
        0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
        0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
        0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
        0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
        0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
        0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
        0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
        0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
        0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
        0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
        0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
        0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16};

    const uint8_t AES256::rsbox[256] = {
        0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
        0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
        0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
        0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
        0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
        0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
        0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
        0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
        0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
        0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
        0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
        0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
        0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
        0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
        0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D};

    const uint8_t AES256::rcon[15] =
        {
            0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D};

    // ================== Utility ==================

    uint8_t AES256::xtime(uint8_t x)
    {
        return (x << 1) ^ ((x & 0x80) ? 0x1B : 0);
    }

    uint8_t AES256::mul(uint8_t a, uint8_t b)
    {
        uint8_t r = 0;
        while (b)
        {
            if (b & 1)
                r ^= a;
            a = xtime(a);
            b >>= 1;
        }
        return r;
    }

    void AES256::inv_sub_bytes(uint8_t *s)
    {
        for (int i = 0; i < 16; ++i)
            s[i] = rsbox[s[i]];
    }

    void AES256::inv_shift_rows(uint8_t *s)
    {
        uint8_t t[16];
        std::memcpy(t, s, 16);

        s[1] = t[13];
        s[5] = t[1];
        s[9] = t[5];
        s[13] = t[9];
        s[2] = t[10];
        s[6] = t[14];
        s[10] = t[2];
        s[14] = t[6];
        s[3] = t[7];
        s[7] = t[11];
        s[11] = t[15];
        s[15] = t[3];
    }

    void AES256::inv_mix_columns(uint8_t *s)
    {
        for (int i = 0; i < 4; ++i)
        {
            uint8_t *c = s + i * 4;
            uint8_t a = c[0], b = c[1], d = c[2], e = c[3];
            c[0] = mul(a, 14) ^ mul(b, 11) ^ mul(d, 13) ^ mul(e, 9);
            c[1] = mul(a, 9) ^ mul(b, 14) ^ mul(d, 11) ^ mul(e, 13);
            c[2] = mul(a, 13) ^ mul(b, 9) ^ mul(d, 14) ^ mul(e, 11);
            c[3] = mul(a, 11) ^ mul(b, 13) ^ mul(d, 9) ^ mul(e, 14);
        }
    }

    // ================== Key Schedule ==================

    void AES256::expand_key()
    {
        std::memcpy(round_key.data(), key.data(), KEY_SIZE);

        int bytes = KEY_SIZE;
        int r = 1;
        uint8_t temp[4];

        while (bytes < 240)
        {
            for (int i = 0; i < 4; ++i)
                temp[i] = round_key[bytes - 4 + i];

            if (bytes % KEY_SIZE == 0)
            {
                uint8_t t = temp[0];
                temp[0] = sbox[temp[1]] ^ rcon[r++];
                temp[1] = sbox[temp[2]];
                temp[2] = sbox[temp[3]];
                temp[3] = sbox[t];
            }
            else if (bytes % KEY_SIZE == 16)
            {
                for (int i = 0; i < 4; ++i)
                    temp[i] = sbox[temp[i]];
            }

            for (int i = 0; i < 4; ++i)
            {
                round_key[bytes] = round_key[bytes - KEY_SIZE] ^ temp[i];
                ++bytes;
            }
        }
    }

    void AES256::decrypt_block(std::array<uint8_t, 16> &block) const
    {
        add_round_key(block.data(), ROUNDS);

        for (int r = ROUNDS - 1; r > 0; --r)
        {
            inv_shift_rows(block.data());
            inv_sub_bytes(block.data());
            add_round_key(block.data(), r);
            inv_mix_columns(block.data());
        }

        inv_shift_rows(block.data());
        inv_sub_bytes(block.data());
        add_round_key(block.data(), 0);
    }

    // ================== Cipher Core ==================

    void AES256::add_round_key(uint8_t *s, int round) const
    {
        for (int i = 0; i < 16; ++i)
            s[i] ^= round_key[round * 16 + i];
    }

    void AES256::sub_bytes(uint8_t *s)
    {
        for (int i = 0; i < 16; ++i)
            s[i] = sbox[s[i]];
    }

    void AES256::shift_rows(uint8_t *s)
    {
        uint8_t t[16];
        std::memcpy(t, s, 16);
        s[1] = t[5];
        s[5] = t[9];
        s[9] = t[13];
        s[13] = t[1];
        s[2] = t[10];
        s[6] = t[14];
        s[10] = t[2];
        s[14] = t[6];
        s[3] = t[15];
        s[7] = t[3];
        s[11] = t[7];
        s[15] = t[11];
    }

    void AES256::mix_columns(uint8_t *s)
    {
        for (int i = 0; i < 4; ++i)
        {
            uint8_t *c = s + i * 4;
            uint8_t a = c[0], b = c[1], d = c[2], e = c[3];
            c[0] = mul(a, 2) ^ mul(b, 3) ^ d ^ e;
            c[1] = a ^ mul(b, 2) ^ mul(d, 3) ^ e;
            c[2] = a ^ b ^ mul(d, 2) ^ mul(e, 3);
            c[3] = mul(a, 3) ^ b ^ d ^ mul(e, 2);
        }
    }

    void AES256::encrypt_block(std::array<uint8_t, 16> &block) const
    {
        add_round_key(block.data(), 0);
        for (int r = 1; r < ROUNDS; ++r)
        {
            sub_bytes(block.data());
            shift_rows(block.data());
            mix_columns(block.data());
            add_round_key(block.data(), r);
        }
        sub_bytes(block.data());
        shift_rows(block.data());
        add_round_key(block.data(), ROUNDS);
    }

    // ================== Padding & RNG ==================

    std::vector<uint8_t> AES256::pad(const std::vector<uint8_t> &in)
    {
        size_t p = BLOCK_SIZE - (in.size() % BLOCK_SIZE);
        auto out = in;
        out.insert(out.end(), p, static_cast<uint8_t>(p));
        return out;
    }

    std::vector<uint8_t> AES256::unpad(const std::vector<uint8_t> &in)
    {
        uint8_t p = in.back();
        if (p == 0 || p > BLOCK_SIZE)
            throw std::runtime_error("Bad padding");
        return {in.begin(), in.end() - p};
    }

    std::array<uint8_t, 16> AES256::random_block()
    {
        std::array<uint8_t, 16> b{};
        std::random_device rd;
        for (auto &v : b)
            v = static_cast<uint8_t>(rd());
        return b;
    }
}
