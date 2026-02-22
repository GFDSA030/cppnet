#pragma once
#include <array>
#include <vector>
#include <cstdint>

namespace fasm::inline crypt
{
    class AES128
    {
    public:
        static constexpr size_t BLOCK_SIZE = 16;

        AES128();

        explicit AES128(const std::vector<uint8_t> &key128);

        std::vector<uint8_t> encrypt(const std::vector<uint8_t> &plain) const;

        std::vector<uint8_t> decrypt(const std::vector<uint8_t> &cipher) const;

    private:
        std::array<uint8_t, 16> key{};
        std::array<uint8_t, 176> round_key{};

        // ================= AES Core =================

        static const uint8_t sbox[256];

        static const uint8_t rsbox[256];

        // ================= 内部処理 =================

        static uint8_t xtime(uint8_t x);

        static uint8_t mul(uint8_t a, uint8_t b);

        void expand_key();

        void add_round_key(uint8_t *state, int round) const;

        void encrypt_block(std::array<uint8_t, 16> &s) const;

        void decrypt_block(std::array<uint8_t, 16> &s) const;

        static void sub_bytes(std::array<uint8_t, 16> &s);

        static void inv_sub_bytes(std::array<uint8_t, 16> &s);

        static void shift_rows(std::array<uint8_t, 16> &s);

        static void inv_shift_rows(std::array<uint8_t, 16> &s);

        static void mix_columns(std::array<uint8_t, 16> &s);

        static void inv_mix_columns(std::array<uint8_t, 16> &s);

        static std::vector<uint8_t> pad(const std::vector<uint8_t> &in);

        static std::vector<uint8_t> unpad(const std::vector<uint8_t> &in);

        static std::array<uint8_t, 16> random_block();
    };
    class AES256
    {
    public:
        static constexpr size_t BLOCK_SIZE = 16;
        static constexpr size_t KEY_SIZE = 32;
        static constexpr int ROUNDS = 14;

        AES256();

        explicit AES256(const std::vector<uint8_t> &key256);

        std::vector<uint8_t> encrypt(const std::vector<uint8_t> &plain) const;

        std::vector<uint8_t> decrypt(const std::vector<uint8_t> &cipher) const;

    private:
        std::array<uint8_t, KEY_SIZE> key{};
        std::array<uint8_t, 240> round_key{}; // 16 * (14+1)

        // ================== AES Tables ==================

        static const uint8_t sbox[256];

        static const uint8_t rsbox[256];

        static const uint8_t rcon[15];

        // ================== Utility ==================

        static uint8_t xtime(uint8_t x);

        static uint8_t mul(uint8_t a, uint8_t b);

        static void inv_sub_bytes(uint8_t *s);

        static void inv_shift_rows(uint8_t *s);

        static void inv_mix_columns(uint8_t *s);

        // ================== Key Schedule ==================

        void expand_key();

        void decrypt_block(std::array<uint8_t, 16> &block) const;

        // ================== Cipher Core ==================

        void add_round_key(uint8_t *s, int round) const;

        static void sub_bytes(uint8_t *s);

        static void shift_rows(uint8_t *s);

        static void mix_columns(uint8_t *s);

        void encrypt_block(std::array<uint8_t, 16> &block) const;

        // ================== Padding & RNG ==================

        static std::vector<uint8_t> pad(const std::vector<uint8_t> &in);

        static std::vector<uint8_t> unpad(const std::vector<uint8_t> &in);

        static std::array<uint8_t, 16> random_block();
    };
}
