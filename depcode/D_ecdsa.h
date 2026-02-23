#pragma once
#if __has_include("boost/multiprecision/cpp_int.hpp")
#include <vector>
#include <cstdint>
#include <string>
#include <boost/multiprecision/cpp_int.hpp>

using boost::multiprecision::cpp_int;

namespace fasm::crypt
{
    class ECDSA
    {
    public:
        ECDSA();

        explicit ECDSA(const std::string &curve_name);

        // 鍵生成
        void generate_keys();

        // 署名（DER形式: SEQUENCE{ r INTEGER, s INTEGER }）
        std::vector<uint8_t> sign(const std::vector<uint8_t> &msg) const;

        // 検証
        bool verify(const std::vector<uint8_t> &msg,
                    const std::vector<uint8_t> &sig) const;

        // PEM 出力（簡易）：DER を base64 化してヘッダを付ける
        std::string export_private_key_pem() const;

        std::string export_public_key_pem() const;

        // インポート関数
        void import_private_key_pem(const std::string &pem);

        void import_public_key_pem(const std::string &pem);

    private:
        // 楕円曲線パラメータ (secp256r1)
        cpp_int p, a, b, n, Gx, Gy;
        // 鍵
        cpp_int d;      // private
        cpp_int Qx, Qy; // public

        struct ECPoint
        {
            cpp_int x, y;
            bool infinity;
        };

        // カーブ初期化 (secp256r1 / P-256)
        void init_curve();

        // ===== ECC 演算（インスタンスメソッドに変更） =====
        ECPoint point_add(const ECPoint &P, const ECPoint &Q) const;

        ECPoint scalar_mul(cpp_int k, ECPoint P) const;

        // ===== ユーティリティ =====
        static cpp_int mod(const cpp_int &x_in, const cpp_int &m);

        cpp_int mod_inverse(cpp_int a_in, const cpp_int &m) const;

        // 乱数: 0 <= ret <= max
        static cpp_int random_bigint(const cpp_int &max);

        // SHA256 の出力（生バイト列）を big integer に変換し n で縮約する
        cpp_int hash_to_int(const std::vector<uint8_t> &msg) const;

        // ===== ASN.1 ヘルパ =====
        static std::vector<uint8_t> asn1_int(const cpp_int &x);

        static std::vector<uint8_t> asn1_seq(const std::vector<uint8_t> &body);

        // 可変個の byte-vector を結合するヘルパ
        static std::vector<uint8_t> concat(const std::vector<std::vector<uint8_t>> &parts);

        // ASN.1 length の読み取り（pos を進める）。返り値は length。例: after tag
        static size_t read_asn1_len_from(const std::vector<uint8_t> &der, size_t &pos);

        // ASN.1 INTEGER の読み取り（pos を進める） - robust
        static cpp_int read_asn1_int(const std::vector<uint8_t> &der, size_t &pos);

        static std::vector<std::string> split_lines(const std::string &s);

        bool is_on_curve(const ECPoint &P) const;

        static std::vector<uint8_t> asn1_oid(const std::vector<uint32_t> &oid);

        static std::vector<uint8_t> asn1_octet_string(const std::vector<uint8_t> &b);

        static std::vector<uint8_t> asn1_bit_string(const std::vector<uint8_t> &b);

        static std::vector<uint8_t> asn1_ctx(uint8_t tag, const std::vector<uint8_t> &body);

        std::vector<uint8_t> to_bytes_fixed(const cpp_int &x, size_t len) const;

        static std::vector<uint8_t> read_asn1_octet_string(const std::vector<uint8_t> &der, size_t &pos);

        static std::vector<uint8_t> read_asn1_bit_string(const std::vector<uint8_t> &der, size_t &pos);

        static cpp_int from_bytes(const std::vector<uint8_t> &b);

        static std::vector<uint8_t> pem_to_der(const std::string &pem);
    };
} // namespace cryptASM
#endif
