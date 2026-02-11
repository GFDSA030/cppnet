#include "ecdsa.h"

#include "sha.h"
#include "baseN.h"

#include <random>
#include <stdexcept>

namespace cryptASM
{
    ECDSA::ECDSA()
    {
        init_curve();
        generate_keys();
    }

    ECDSA::ECDSA(const std::string &curve_name)
    {
        (void)curve_name; // 現状は secp256r1 固定
        init_curve();
        generate_keys();
    }

    // 鍵生成
    void ECDSA::generate_keys()
    {
        // d in [1, n-1]
        d = random_bigint(n - 1) + 1;
        ECPoint G{Gx, Gy, false};
        ECPoint Q = scalar_mul(d, G);
        if (Q.infinity)
            throw std::runtime_error("bad generated public point");
        Qx = Q.x;
        Qy = Q.y;
    }

    // 署名（DER形式: SEQUENCE{ r INTEGER, s INTEGER }）
    std::vector<uint8_t> ECDSA::sign(const std::vector<uint8_t> &msg) const
    {
        cpp_int z = hash_to_int(msg);
        cpp_int r, s;

        ECPoint G{Gx, Gy, false};

        while (true)
        {
            cpp_int k = random_bigint(n - 1) + 1;
            ECPoint P = scalar_mul(k, G);
            r = mod(P.x, n);
            if (r == 0)
                continue;
            s = mod(mod_inverse(k, n) * (z + r * d), n);
            if (s == 0)
                continue;
            break;
        }

        auto R = asn1_int(r);
        auto S = asn1_int(s);
        auto seq = asn1_seq(concat({R, S}));
        return seq;
    }

    // 検証
    bool ECDSA::verify(const std::vector<uint8_t> &msg,
                       const std::vector<uint8_t> &sig) const
    {
        size_t pos = 0;
        // parse SEQUENCE
        if (pos >= sig.size() || sig[pos++] != 0x30)
            return false;
        size_t seqlen = read_asn1_len_from(sig, pos);
        if (pos + seqlen > sig.size())
            return false;

        cpp_int r = read_asn1_int(sig, pos);
        cpp_int s = read_asn1_int(sig, pos);

        if (r <= 0 || r >= n || s <= 0 || s >= n)
            return false;

        cpp_int z = hash_to_int(msg);
        cpp_int w = mod_inverse(s, n);
        cpp_int u1 = mod(z * w, n);
        cpp_int u2 = mod(r * w, n);

        ECPoint G{Gx, Gy, false};
        ECPoint Q{Qx, Qy, false};

        ECPoint P1 = scalar_mul(u1, G);
        ECPoint P2 = scalar_mul(u2, Q);
        ECPoint P = point_add(P1, P2);

        if (P.infinity)
            return false;
        return mod(P.x, n) == r;
    }

    // PEM 出力（簡易）：DER を base64 化してヘッダを付ける
    // std::string ECDSA::export_private_key_pem() const
    // {
    //     // 非標準の簡易フォーマット: SEQ( version=1, d, Qx, Qy )
    //     auto seq_parts = std::vector<std::vector<uint8_t>>{
    //         asn1_int(1),
    //         asn1_int(d),
    //         asn1_int(Qx),
    //         asn1_int(Qy)};
    //     auto body = asn1_seq(concat(seq_parts));
    //     std::string der(body.begin(), body.end());
    //     std::string b64 = base64_encode(der, der.size());

    //     return "-----BEGIN ECDSA PRIVATE KEY-----\n" + b64 + "\n-----END ECDSA PRIVATE KEY-----";
    // }

    // std::string ECDSA::export_public_key_pem() const
    // {
    //     // 非標準の簡易フォーマット: SEQ( Qx, Qy )
    //     auto seq_parts = std::vector<std::vector<uint8_t>>{
    //         asn1_int(Qx),
    //         asn1_int(Qy)};
    //     auto body = asn1_seq(concat(seq_parts));
    //     std::string der(body.begin(), body.end());
    //     std::string b64 = base64_encode(der, der.size());

    //     return "-----BEGIN ECDSA PUBLIC KEY-----\n" + b64 + "\n-----END ECDSA PUBLIC KEY-----";
    // }

    std::string ECDSA::export_public_key_pem() const
    {
        // ecPublicKey OID + secp256r1 OID
        auto alg = asn1_seq(concat({asn1_oid({1, 2, 840, 10045, 2, 1}),
                                    asn1_oid({1, 2, 840, 10045, 3, 1, 7})}));

        // 04 || X || Y
        std::vector<uint8_t> pub;
        pub.push_back(0x04);
        auto xb = to_bytes_fixed(Qx, 32);
        auto yb = to_bytes_fixed(Qy, 32);
        pub.insert(pub.end(), xb.begin(), xb.end());
        pub.insert(pub.end(), yb.begin(), yb.end());

        auto spk = asn1_bit_string(pub);

        auto seq = asn1_seq(concat({alg, spk}));

        std::string der(seq.begin(), seq.end());
        std::string b64 = base64_encode(der, der.size());

        return "-----BEGIN PUBLIC KEY-----\n" + b64 +
               "\n-----END PUBLIC KEY-----";
    }

    std::string ECDSA::export_private_key_pem() const
    {
        // privateKey OCTET STRING
        auto dbytes = to_bytes_fixed(d, 32);

        auto body = std::vector<std::vector<uint8_t>>{
            asn1_int(1),
            asn1_octet_string(dbytes),
            asn1_ctx(0, asn1_oid({1, 2, 840, 10045, 3, 1, 7}))};

        // public key [1]
        std::vector<uint8_t> pub;
        pub.push_back(0x04);
        auto xb = to_bytes_fixed(Qx, 32);
        auto yb = to_bytes_fixed(Qy, 32);
        pub.insert(pub.end(), xb.begin(), xb.end());
        pub.insert(pub.end(), yb.begin(), yb.end());

        body.push_back(asn1_ctx(1, asn1_bit_string(pub)));

        auto seq = asn1_seq(concat(body));

        std::string der(seq.begin(), seq.end());
        std::string b64 = base64_encode(der, der.size());

        return "-----BEGIN EC PRIVATE KEY-----\n" + b64 +
               "\n-----END EC PRIVATE KEY-----";
    }

    // void cryptASM::ECDSA::import_private_key_pem(const std::string &pem)
    // {
    //     // PEM → DER
    //     std::string b64;
    //     for (const auto &line : split_lines(pem))
    //     {
    //         if (line.find("-----") != std::string::npos)
    //             continue;
    //         b64 += line;
    //     }

    //     std::string der_str = base64_decode(b64, b64.size());

    //     std::vector<uint8_t> der(der_str.begin(), der_str.end());

    //     size_t pos = 0;

    //     // SEQUENCE
    //     if (pos >= der.size() || der[pos++] != 0x30)
    //         throw std::runtime_error("invalid ECDSA private key (no SEQUENCE)");

    //     size_t seqlen = read_asn1_len_from(der, pos);
    //     if (pos + seqlen > der.size())
    //         throw std::runtime_error("invalid ECDSA private key length");

    //     cpp_int version = read_asn1_int(der, pos);
    //     if (version != 1)
    //         throw std::runtime_error("unsupported ECDSA private key version");

    //     d = read_asn1_int(der, pos);
    //     Qx = read_asn1_int(der, pos);
    //     Qy = read_asn1_int(der, pos);

    //     // sanity check
    //     if (d <= 0 || d >= n)
    //         throw std::runtime_error("invalid private key scalar");

    //     ECPoint Q{Qx, Qy, false};
    //     if (!is_on_curve(Q))
    //         throw std::runtime_error("public key not on curve");
    // }

    // void cryptASM::ECDSA::import_public_key_pem(const std::string &pem)
    // {
    //     std::string b64;
    //     for (const auto &line : split_lines(pem))
    //     {
    //         if (line.find("-----") != std::string::npos)
    //             continue;
    //         b64 += line;
    //     }

    //     std::string der_str = base64_decode(b64, b64.size());

    //     std::vector<uint8_t> der(der_str.begin(), der_str.end());

    //     size_t pos = 0;

    //     // SEQUENCE
    //     if (pos >= der.size() || der[pos++] != 0x30)
    //         throw std::runtime_error("invalid ECDSA public key (no SEQUENCE)");

    //     size_t seqlen = read_asn1_len_from(der, pos);
    //     if (pos + seqlen > der.size())
    //         throw std::runtime_error("invalid ECDSA public key length");

    //     Qx = read_asn1_int(der, pos);
    //     Qy = read_asn1_int(der, pos);

    //     ECPoint Q{Qx, Qy, false};
    //     if (!is_on_curve(Q))
    //         throw std::runtime_error("public key not on curve");
    // }

    void ECDSA::import_private_key_pem(const std::string &pem)
    {
        auto der = pem_to_der(pem);
        size_t pos = 0;

        if (der[pos++] != 0x30)
            throw std::runtime_error("Invalid EC private key");

        read_asn1_len_from(der, pos);

        // version
        read_asn1_int(der, pos);

        // privateKey OCTET STRING
        auto priv = read_asn1_octet_string(der, pos);
        d = from_bytes(priv);

        // optional fields
        while (pos < der.size())
        {
            uint8_t tag = der[pos++];
            size_t len = read_asn1_len_from(der, pos);

            // [1] publicKey
            if (tag == 0xA1)
            {
                size_t p2 = pos;
                auto bits = read_asn1_bit_string(der, p2);

                if (bits[0] != 0x04)
                    throw std::runtime_error("Invalid EC point");

                size_t coord = (bits.size() - 1) / 2;
                Qx = from_bytes({bits.begin() + 1, bits.begin() + 1 + coord});
                Qy = from_bytes({bits.begin() + 1 + coord, bits.end()});
            }

            pos += len;
        }
    }

    void ECDSA::import_public_key_pem(const std::string &pem)
    {
        auto der = pem_to_der(pem);
        size_t pos = 0;

        if (der[pos++] != 0x30)
            throw std::runtime_error("Invalid public key");

        read_asn1_len_from(der, pos);

        // AlgorithmIdentifier
        if (der[pos++] != 0x30)
            throw std::runtime_error("Invalid algorithm identifier");

        read_asn1_len_from(der, pos);

        // skip OIDs
        while (der[pos] == 0x06)
        {
            pos++;
            size_t len = read_asn1_len_from(der, pos);
            pos += len;
        }

        // BIT STRING
        auto bits = read_asn1_bit_string(der, pos);

        if (bits[0] != 0x04)
            throw std::runtime_error("Invalid EC point");

        size_t coord = (bits.size() - 1) / 2;
        Qx = from_bytes({bits.begin() + 1, bits.begin() + 1 + coord});
        Qy = from_bytes({bits.begin() + 1 + coord, bits.end()});
    }

    std::vector<std::string> ECDSA::split_lines(const std::string &s)
    {
        std::vector<std::string> lines;
        std::string cur;
        for (char c : s)
        {
            if (c == '\n')
            {
                if (!cur.empty())
                    lines.push_back(cur);
                cur.clear();
            }
            else if (c != '\r')
            {
                cur.push_back(c);
            }
        }
        if (!cur.empty())
            lines.push_back(cur);
        return lines;
    }

    bool ECDSA::is_on_curve(const ECDSA::ECPoint &P) const
    {
        if (P.infinity)
            return false;

        // y^2 = x^3 + ax + b (mod p)
        cpp_int lhs = mod(P.y * P.y, p);
        cpp_int rhs = mod(P.x * P.x * P.x + a * P.x + b, p);
        return lhs == rhs;
    }

    // カーブ初期化 (secp256r1 / P-256)
    void ECDSA::init_curve()
    {
        p = cpp_int("0xffffffff00000001000000000000000000000000ffffffffffffffffffffffff");
        // a = p - 3  (we can represent as -3 and use mod(...) in ops)
        a = cpp_int("-3");
        b = cpp_int("0x5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b");
        n = cpp_int("0xffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551");
        Gx = cpp_int("0x6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296");
        Gy = cpp_int("0x4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5");
    }

    // ===== ECC 演算（インスタンスメソッドに変更） =====
    ECDSA::ECPoint ECDSA::point_add(const ECPoint &P, const ECPoint &Q) const
    {
        if (P.infinity)
            return Q;
        if (Q.infinity)
            return P;
        if (P.x == Q.x && P.y != Q.y)
            return ECPoint{0, 0, true};

        cpp_int lambda;
        if (P.x == Q.x && P.y == Q.y)
        {
            // λ = (3 x^2 + a) / (2 y)
            cpp_int num = (cpp_int(3) * P.x * P.x + a);
            cpp_int den = mod_inverse(mod(cpp_int(2) * P.y, p), p);
            lambda = mod(num * den, p);
        }
        else
        {
            // λ = (y2 - y1) / (x2 - x1)
            cpp_int num = mod(Q.y - P.y, p);
            cpp_int den = mod_inverse(mod(Q.x - P.x, p), p);
            lambda = mod(num * den, p);
        }

        cpp_int xr = mod(lambda * lambda - P.x - Q.x, p);
        cpp_int yr = mod(lambda * (P.x - xr) - P.y, p);
        return ECPoint{xr, yr, false};
    }

    ECDSA::ECPoint ECDSA::scalar_mul(cpp_int k, ECPoint P) const
    {
        ECPoint R{0, 0, true};
        // simple double-and-add (left-to-right)
        // ensure k >= 0
        if (k < 0)
            k = mod(k, n);
        while (k > 0)
        {
            if ((k & 1) != 0)
                R = point_add(R, P);
            P = point_add(P, P);
            k >>= 1;
        }
        return R;
    }

    // ===== ユーティリティ =====
    cpp_int ECDSA::mod(const cpp_int &x_in, const cpp_int &m)
    {
        cpp_int x = x_in % m;
        if (x < 0)
            x += m;
        return x;
    }

    cpp_int ECDSA::mod_inverse(cpp_int a_in, const cpp_int &m) const
    {
        cpp_int a = mod(a_in, m);
        cpp_int t = 0, newt = 1;
        cpp_int r = m, newr = a;
        while (newr != 0)
        {
            cpp_int q = r / newr;
            cpp_int tmp;
            tmp = newt;
            newt = t - q * newt;
            t = tmp;
            tmp = newr;
            newr = r - q * newr;
            r = tmp;
        }
        if (r > 1)
            throw std::runtime_error("no inverse");
        if (t < 0)
            t += m;
        return t;
    }

    // 乱数: 0 <= ret <= max
    cpp_int ECDSA::random_bigint(const cpp_int &max)
    {
        if (max <= 0)
            return 0;
        // generate using 64-bit blocks
        std::random_device rd;
        std::mt19937_64 eng(rd());
        std::uniform_int_distribution<uint64_t> dist;

        // determine how many 64-bit words needed
        // rough bit-length:
        size_t bits = 0;
        cpp_int tmp = max;
        while (tmp > 0)
        {
            tmp >>= 1;
            ++bits;
        }
        size_t words = (bits + 63) / 64;

        cpp_int r = 0;
        for (size_t i = 0; i < words; ++i)
        {
            r <<= 64;
            r |= dist(eng);
        }
        return r % (max + 1);
    }

    // SHA256 の出力（生バイト列）を big integer に変換し n で縮約する
    cpp_int ECDSA::hash_to_int(const std::vector<uint8_t> &msg) const
    {
        std::string s(msg.begin(), msg.end());
        std::string h = SHA256::hash256(s); // 生バイト列である前提
        // h.size() == 32 を想定
        cpp_int z = 0;
        for (unsigned char c : h)
        {
            z <<= 8;
            z |= (cpp_int)(uint8_t)c;
        }
        // If h is longer than order size, truncate per RFC; here mod n suffices.
        return z % n;
    }

    // ===== ASN.1 ヘルパ =====
    std::vector<uint8_t> ECDSA::asn1_int(const cpp_int &x)
    {
        cpp_int v = x;
        // produce big-endian bytes
        std::vector<uint8_t> b;
        if (v == 0)
        {
            b.push_back(0);
        }
        else
        {
            cpp_int t = v;
            while (t > 0)
            {
                uint8_t byte = static_cast<uint8_t>((t & 0xFF));
                b.insert(b.begin(), byte);
                t >>= 8;
            }
        }
        // ensure positive (highest bit zero)
        if (b.empty() || (b[0] & 0x80))
            b.insert(b.begin(), 0);
        // INTEGER tag
        std::vector<uint8_t> out;
        out.push_back(0x02);
        // length (short form assumed; values here are small)
        if (b.size() < 0x80)
        {
            out.push_back(static_cast<uint8_t>(b.size()));
        }
        else
        {
            // long form (unlikely for integers used here, but implement)
            size_t len = b.size();
            std::vector<uint8_t> lenbytes;
            while (len > 0)
            {
                lenbytes.insert(lenbytes.begin(), uint8_t(len & 0xFF));
                len >>= 8;
            }
            out.push_back(0x80 | static_cast<uint8_t>(lenbytes.size()));
            out.insert(out.end(), lenbytes.begin(), lenbytes.end());
        }
        out.insert(out.end(), b.begin(), b.end());
        return out;
    }

    std::vector<uint8_t> ECDSA::asn1_seq(const std::vector<uint8_t> &body)
    {
        std::vector<uint8_t> out;
        out.push_back(0x30);
        size_t len = body.size();
        if (len < 0x80)
        {
            out.push_back(static_cast<uint8_t>(len));
        }
        else
        {
            std::vector<uint8_t> lenbytes;
            while (len > 0)
            {
                lenbytes.insert(lenbytes.begin(), uint8_t(len & 0xFF));
                len >>= 8;
            }
            out.push_back(0x80 | static_cast<uint8_t>(lenbytes.size()));
            out.insert(out.end(), lenbytes.begin(), lenbytes.end());
        }
        out.insert(out.end(), body.begin(), body.end());
        return out;
    }

    // 可変個の byte-vector を結合するヘルパ
    std::vector<uint8_t> ECDSA::concat(const std::vector<std::vector<uint8_t>> &parts)
    {
        std::vector<uint8_t> out;
        size_t total = 0;
        for (const auto &p : parts)
            total += p.size();
        out.reserve(total);
        for (const auto &p : parts)
            out.insert(out.end(), p.begin(), p.end());
        return out;
    }

    // ASN.1 length の読み取り（pos を進める）。返り値は length。例: after tag
    size_t ECDSA::read_asn1_len_from(const std::vector<uint8_t> &der, size_t &pos)
    {
        if (pos >= der.size())
            throw std::runtime_error("asn1 length OOB");
        uint8_t first = der[pos++];
        if ((first & 0x80) == 0)
        {
            // short form
            return first;
        }
        else
        {
            size_t nbytes = first & 0x7F;
            if (nbytes == 0 || nbytes > sizeof(size_t))
                throw std::runtime_error("unsupported ASN.1 length");
            if (pos + nbytes > der.size())
                throw std::runtime_error("asn1 length OOB");
            size_t len = 0;
            for (size_t i = 0; i < nbytes; ++i)
            {
                len = (len << 8) | der[pos++];
            }
            return len;
        }
    }

    // ASN.1 INTEGER の読み取り（pos を進める） - robust
    cpp_int ECDSA::read_asn1_int(const std::vector<uint8_t> &der, size_t &pos)
    {
        if (pos >= der.size() || der[pos++] != 0x02)
            throw std::runtime_error("expected INTEGER");
        size_t len = read_asn1_len_from(der, pos);
        if (pos + len > der.size())
            throw std::runtime_error("INTEGER OOB");
        cpp_int v = 0;
        for (size_t i = 0; i < len; ++i)
        {
            v <<= 8;
            v |= (uint8_t)der[pos++];
        }
        return v;
    }

    std::vector<uint8_t> ECDSA::asn1_oid(const std::vector<uint32_t> &oid)
    {
        if (oid.size() < 2)
            throw std::runtime_error("OID too short");

        std::vector<uint8_t> body;
        body.push_back(static_cast<uint8_t>(oid[0] * 40 + oid[1]));

        for (size_t i = 2; i < oid.size(); ++i)
        {
            uint32_t v = oid[i];
            std::vector<uint8_t> tmp;
            do
            {
                tmp.insert(tmp.begin(), (v & 0x7F) | (tmp.empty() ? 0 : 0x80));
                v >>= 7;
            } while (v > 0);
            body.insert(body.end(), tmp.begin(), tmp.end());
        }

        std::vector<uint8_t> out = {0x06};
        out.push_back(static_cast<uint8_t>(body.size()));
        out.insert(out.end(), body.begin(), body.end());
        return out;
    }

    std::vector<uint8_t> ECDSA::asn1_octet_string(const std::vector<uint8_t> &b)
    {
        std::vector<uint8_t> out = {0x04, (uint8_t)b.size()};
        out.insert(out.end(), b.begin(), b.end());
        return out;
    }

    std::vector<uint8_t> ECDSA::asn1_bit_string(const std::vector<uint8_t> &b)
    {
        std::vector<uint8_t> out = {0x03, (uint8_t)(b.size() + 1), 0x00};
        out.insert(out.end(), b.begin(), b.end());
        return out;
    }

    std::vector<uint8_t> ECDSA::asn1_ctx(uint8_t tag, const std::vector<uint8_t> &body)
    {
        std::vector<uint8_t> out = {static_cast<uint8_t>(0xA0 | tag),
                                    static_cast<uint8_t>(body.size())};
        out.insert(out.end(), body.begin(), body.end());
        return out;
    }
    std::vector<uint8_t> ECDSA::to_bytes_fixed(const cpp_int &x, size_t len) const
    {
        if (x < 0)
            throw std::runtime_error("to_bytes_fixed: negative integer");

        std::vector<uint8_t> out(len, 0);

        cpp_int v = x;
        size_t i = len;

        while (v > 0)
        {
            if (i == 0)
                throw std::runtime_error("to_bytes_fixed: integer too large");

            out[--i] = static_cast<uint8_t>(v & 0xFF);
            v >>= 8;
        }

        return out;
    }

    std::vector<uint8_t> ECDSA::read_asn1_octet_string(
        const std::vector<uint8_t> &der, size_t &pos)
    {
        if (der[pos++] != 0x04)
            throw std::runtime_error("Expected OCTET STRING");

        size_t len = read_asn1_len_from(der, pos);
        std::vector<uint8_t> out(der.begin() + pos, der.begin() + pos + len);
        pos += len;
        return out;
    }

    std::vector<uint8_t> ECDSA::read_asn1_bit_string(
        const std::vector<uint8_t> &der, size_t &pos)
    {
        if (der[pos++] != 0x03)
            throw std::runtime_error("Expected BIT STRING");

        size_t len = read_asn1_len_from(der, pos);
        pos++; // unused bits count
        std::vector<uint8_t> out(der.begin() + pos, der.begin() + pos + len - 1);
        pos += len - 1;
        return out;
    }

    cpp_int ECDSA::from_bytes(const std::vector<uint8_t> &b)
    {
        cpp_int x = 0;
        for (uint8_t v : b)
        {
            x <<= 8;
            x |= v;
        }
        return x;
    }

    std::vector<uint8_t> ECDSA::pem_to_der(const std::string &pem)
    {
        std::string b64;
        b64.reserve(pem.size());

        bool in_body = false;

        for (size_t i = 0; i < pem.size();)
        {
            // 行単位で処理
            size_t end = pem.find('\n', i);
            if (end == std::string::npos)
                end = pem.size();

            std::string line = pem.substr(i, end - i);
            i = end + 1;

            if (line.find("-----BEGIN ") != std::string::npos)
            {
                in_body = true;
                continue;
            }
            if (line.find("-----END ") != std::string::npos)
                break;

            if (!in_body)
                continue;

            for (char c : line)
            {
                if (std::isalnum(static_cast<unsigned char>(c)) ||
                    c == '+' || c == '/' || c == '=')
                {
                    b64.push_back(c);
                }
            }
        }

        if (b64.empty())
            throw std::runtime_error("pem_to_der: no base64 data");

        std::string decoded = base64_decode(b64, b64.size());

        return std::vector<uint8_t>(decoded.begin(), decoded.end());
    }
} // namespace cryptASM
