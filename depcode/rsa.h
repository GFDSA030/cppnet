#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <boost/multiprecision/cpp_int.hpp>

using boost::multiprecision::cpp_int;
namespace cryptASM
{
    class RSA
    {
    public:
        explicit RSA() : e(65537)
        {
        }
        explicit RSA(size_t bits) : e(65537)
        {
            if (bits > 0)
                generate_keys(bits);
        }

        // Encrypt / Decrypt
        std::vector<uint8_t> encrypt(const std::vector<uint8_t> &msg) const;

        std::vector<uint8_t> decrypt(const std::vector<uint8_t> &cipher) const;

        // PEM Export
        std::string export_public_key_pem() const;

        std::string export_private_key_pem() const;

        // --- 公開鍵インポート（SEQUENCEヘッダをスキップする） ---
        void import_public_key_pem(const std::string &pem);

        // --- 秘密鍵インポート（SEQUENCEヘッダをスキップし9個のINTEGERを読む） ---
        void import_private_key_pem(const std::string &pem);

        // ========== 鍵生成 ==========
        void generate_keys(size_t bits = 2048);

    private:
        cpp_int n, e, d, p, q, dp, dq, qi;

        // ========== 基本演算 ==========
        static cpp_int mod_pow(cpp_int b, cpp_int e, const cpp_int &m);

        static cpp_int ext_gcd(cpp_int a, cpp_int b, cpp_int &x, cpp_int &y);

        static cpp_int mod_inverse(const cpp_int &a, const cpp_int &m);

        // ========== 鍵生成 ==========
        static cpp_int random_bigint(size_t bits);

        static bool miller_rabin(const cpp_int &n, int k = 40);

        static cpp_int gen_prime(size_t bits);

        // ========== OAEP ==========
        static std::vector<uint8_t> sha256_bytes(const std::vector<uint8_t> &in);

        static std::vector<uint8_t> mgf1(const std::vector<uint8_t> &seed, size_t len);

        cpp_int oaep_encode(const std::vector<uint8_t> &msg) const;

        static std::vector<uint8_t> oaep_decode(const std::vector<uint8_t> &EM);

        // ========== バイト変換 / ASN.1 ==========
        static cpp_int from_bytes(const std::vector<uint8_t> &b);

        std::vector<uint8_t> to_bytes_fixed(cpp_int x) const;

        static std::vector<uint8_t> asn1_len(size_t len);

        static std::vector<uint8_t> asn1_int(const cpp_int &x);

        static std::vector<uint8_t> asn1_seq(const std::vector<uint8_t> &body);

        static std::string base64_encode_vec(const std::vector<uint8_t> &in);

        // --- PEM -> DER (robust): remove header/footer lines and join base64 lines ---
        static std::vector<uint8_t> pem_to_der(const std::string &pem);

        // --- ASN.1 length reader (advances pos) ---
        static size_t read_asn1_len_from(const std::vector<uint8_t> &der, size_t &pos);

        // --- ASN.1 INTEGER reader (robust) ---
        static cpp_int read_asn1_int(const std::vector<uint8_t> &der, size_t &pos);
    };
}
