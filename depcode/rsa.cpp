#include "rsa.h"

#include "sha.h"
#include "baseN.h"

#include <random>
#include <stdexcept>

namespace cryptASM
{
    // Encrypt / Decrypt
    std::vector<uint8_t> RSA::encrypt(const std::vector<uint8_t> &msg) const
    {
        cpp_int m = oaep_encode(msg);
        cpp_int c = mod_pow(m, e, n);
        return to_bytes_fixed(c);
    }

    std::vector<uint8_t> RSA::decrypt(const std::vector<uint8_t> &cipher) const
    {
        cpp_int c = from_bytes(cipher);
        cpp_int m = mod_pow(c, d, n);
        return oaep_decode(to_bytes_fixed(m));
    }

    // PEM Export
    std::string RSA::export_public_key_pem() const
    {
        std::vector<uint8_t> body;
        auto vn = asn1_int(n);
        auto ve = asn1_int(e);
        body.insert(body.end(), vn.begin(), vn.end());
        body.insert(body.end(), ve.begin(), ve.end());
        auto der = asn1_seq(body);
        auto b64 = base64_encode_vec(der);

        std::ostringstream oss;
        oss << "-----BEGIN RSA PUBLIC KEY-----\n";
        for (size_t i = 0; i < b64.size(); i += 64)
            oss << b64.substr(i, 64) << "\n";
        oss << "-----END RSA PUBLIC KEY-----\n";
        return oss.str();
    }

    std::string RSA::export_private_key_pem() const
    {
        std::vector<uint8_t> body;
        auto v0 = asn1_int(0);
        auto vn = asn1_int(n);
        auto ve = asn1_int(e);
        auto vd = asn1_int(d);
        auto vp = asn1_int(p);
        auto vq = asn1_int(q);
        auto vdp = asn1_int(dp);
        auto vdq = asn1_int(dq);
        auto vqi = asn1_int(qi);

        body.insert(body.end(), v0.begin(), v0.end());
        body.insert(body.end(), vn.begin(), vn.end());
        body.insert(body.end(), ve.begin(), ve.end());
        body.insert(body.end(), vd.begin(), vd.end());
        body.insert(body.end(), vp.begin(), vp.end());
        body.insert(body.end(), vq.begin(), vq.end());
        body.insert(body.end(), vdp.begin(), vdp.end());
        body.insert(body.end(), vdq.begin(), vdq.end());
        body.insert(body.end(), vqi.begin(), vqi.end());

        auto der = asn1_seq(body);
        auto b64 = base64_encode_vec(der);

        std::ostringstream oss;
        oss << "-----BEGIN RSA PRIVATE KEY-----\n";
        for (size_t i = 0; i < b64.size(); i += 64)
            oss << b64.substr(i, 64) << "\n";
        oss << "-----END RSA PRIVATE KEY-----\n";
        return oss.str();
    }

    // --- 公開鍵インポート（SEQUENCEヘッダをスキップする） ---
    void RSA::import_public_key_pem(const std::string &pem)
    {
        std::vector<uint8_t> der = pem_to_der(pem);
        size_t pos = 0;
        if (pos >= der.size() || der[pos++] != 0x30)
            throw std::runtime_error("ASN1 seq error");
        size_t seqlen = read_asn1_len_from(der, pos);
        size_t seq_end = pos + seqlen;

        n = read_asn1_int(der, pos);
        e = read_asn1_int(der, pos);

        if (pos != seq_end)
        {
            // 余分なデータがあれば無視（許容）
            // throw std::runtime_error("extra data in public key"); // optional strict check
        }
    }

    // --- 秘密鍵インポート（SEQUENCEヘッダをスキップし9個のINTEGERを読む） ---
    void RSA::import_private_key_pem(const std::string &pem)
    {
        std::vector<uint8_t> der = pem_to_der(pem);
        size_t pos = 0;
        if (pos >= der.size() || der[pos++] != 0x30)
            throw std::runtime_error("ASN1 seq error");
        size_t seqlen = read_asn1_len_from(der, pos);
        size_t seq_end = pos + seqlen;

        cpp_int v0 = read_asn1_int(der, pos); // version
        n = read_asn1_int(der, pos);
        e = read_asn1_int(der, pos);
        d = read_asn1_int(der, pos);
        p = read_asn1_int(der, pos);
        q = read_asn1_int(der, pos);
        dp = read_asn1_int(der, pos);
        dq = read_asn1_int(der, pos);
        qi = read_asn1_int(der, pos);

        if (pos != seq_end)
        {
            // 余分なデータがあれば無視。必要ならここで検査する。
        }
    }

    // ========== 基本演算 ==========
    cpp_int RSA::mod_pow(cpp_int b, cpp_int e, const cpp_int &m)
    {
        cpp_int r = 1;
        b %= m;
        while (e > 0)
        {
            if (e & 1)
                r = (r * b) % m;
            b = (b * b) % m;
            e >>= 1;
        }
        return r;
    }

    cpp_int RSA::ext_gcd(cpp_int a, cpp_int b, cpp_int &x, cpp_int &y)
    {
        if (b == 0)
        {
            x = 1;
            y = 0;
            return a;
        }
        cpp_int x1, y1;
        cpp_int g = ext_gcd(b, a % b, x1, y1);
        x = y1;
        y = x1 - (a / b) * y1;
        return g;
    }

    cpp_int RSA::mod_inverse(const cpp_int &a, const cpp_int &m)
    {
        cpp_int x, y;
        if (ext_gcd(a, m, x, y) != 1)
            throw std::runtime_error("no inverse");
        return (x % m + m) % m;
    }

    // ========== 鍵生成 ==========
    cpp_int RSA::random_bigint(size_t bits)
    {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        cpp_int r = 0;
        for (size_t i = 0; i < bits; i += 64)
        {
            r <<= 64;
            r |= gen();
        }
        r |= cpp_int(1) << (bits - 1);
        r |= 1;
        return r;
    }

    bool RSA::miller_rabin(const cpp_int &n, int k)
    {
        if (n < 4)
            return n == 2 || n == 3;
        if (n % 2 == 0)
            return false;
        cpp_int d = n - 1;
        int s = 0;
        while ((d & 1) == 0)
        {
            d >>= 1;
            s++;
        }
        std::random_device rd;
        std::mt19937_64 gen(rd());
        for (int i = 0; i < k; i++)
        {
            cpp_int a = 2 + cpp_int(gen()) % (n - 3);
            cpp_int x = mod_pow(a, d, n);
            if (x == 1 || x == n - 1)
                continue;
            bool comp = true;
            for (int r = 1; r < s; r++)
            {
                x = mod_pow(x, 2, n);
                if (x == n - 1)
                {
                    comp = false;
                    break;
                }
            }
            if (comp)
                return false;
        }
        return true;
    }

    cpp_int RSA::gen_prime(size_t bits)
    {
        while (true)
        {
            cpp_int p = random_bigint(bits);
            if (miller_rabin(p))
                return p;
        }
    }

    void RSA::generate_keys(size_t bits)
    {
        p = gen_prime(bits / 2);
        q = gen_prime(bits / 2);
        if (p < q)
            std::swap(p, q);
        n = p * q;
        cpp_int phi = (p - 1) * (q - 1);
        d = mod_inverse(e, phi);
        dp = d % (p - 1);
        dq = d % (q - 1);
        qi = mod_inverse(q, p);
    }

    // ========== OAEP ==========
    std::vector<uint8_t> RSA::sha256_bytes(const std::vector<uint8_t> &in)
    {
        std::string s(in.begin(), in.end());
        std::string hex = SHA256::str256(s);
        std::vector<uint8_t> out;
        for (size_t i = 0; i < hex.size(); i += 2)
            out.push_back(std::stoi(hex.substr(i, 2), nullptr, 16));
        return out;
    }

    std::vector<uint8_t> RSA::mgf1(const std::vector<uint8_t> &seed, size_t len)
    {
        std::vector<uint8_t> mask;
        for (uint32_t c = 0; mask.size() < len; c++)
        {
            std::vector<uint8_t> t = seed;
            t.push_back((c >> 24) & 0xFF);
            t.push_back((c >> 16) & 0xFF);
            t.push_back((c >> 8) & 0xFF);
            t.push_back(c & 0xFF);
            auto h = sha256_bytes(t);
            mask.insert(mask.end(), h.begin(), h.end());
        }
        mask.resize(len);
        return mask;
    }

    cpp_int RSA::oaep_encode(const std::vector<uint8_t> &msg) const
    {
        const size_t hLen = 32;
        size_t k = (msb(n) + 8) / 8;
        if (msg.size() > k - 2 * hLen - 2)
            throw std::runtime_error("message too long");
        std::vector<uint8_t> lHash = sha256_bytes({});
        std::vector<uint8_t> PS(k - msg.size() - 2 * hLen - 2, 0x00);
        std::vector<uint8_t> DB = lHash;
        DB.insert(DB.end(), PS.begin(), PS.end());
        DB.push_back(0x01);
        DB.insert(DB.end(), msg.begin(), msg.end());
        std::vector<uint8_t> seed(hLen);
        std::random_device rd;
        for (auto &b : seed)
            b = rd();
        auto dbMask = mgf1(seed, DB.size());
        for (size_t i = 0; i < DB.size(); i++)
            DB[i] ^= dbMask[i];
        auto seedMask = mgf1(DB, hLen);
        for (size_t i = 0; i < hLen; i++)
            seed[i] ^= seedMask[i];
        std::vector<uint8_t> EM = {0x00};
        EM.insert(EM.end(), seed.begin(), seed.end());
        EM.insert(EM.end(), DB.begin(), DB.end());
        return from_bytes(EM);
    }

    std::vector<uint8_t> RSA::oaep_decode(const std::vector<uint8_t> &EM)
    {
        const size_t hLen = 32;
        if (EM.size() < 2 * hLen + 2 || EM[0] != 0x00)
            throw std::runtime_error("oaep error");
        std::vector<uint8_t> maskedSeed(EM.begin() + 1, EM.begin() + 1 + hLen);
        std::vector<uint8_t> maskedDB(EM.begin() + 1 + hLen, EM.end());
        auto seedMask = mgf1(maskedDB, hLen);
        for (size_t i = 0; i < hLen; i++)
            maskedSeed[i] ^= seedMask[i];
        auto dbMask = mgf1(maskedSeed, maskedDB.size());
        for (size_t i = 0; i < maskedDB.size(); i++)
            maskedDB[i] ^= dbMask[i];
        size_t i = hLen;
        while (i < maskedDB.size() && maskedDB[i] == 0x00)
            i++;
        if (i == maskedDB.size() || maskedDB[i] != 0x01)
            throw std::runtime_error("oaep error");
        return std::vector<uint8_t>(maskedDB.begin() + i + 1, maskedDB.end());
    }

    // ========== バイト変換 / ASN.1 ==========
    cpp_int RSA::from_bytes(const std::vector<uint8_t> &b)
    {
        cpp_int x = 0;
        for (uint8_t v : b)
        {
            x <<= 8;
            x |= v;
        }
        return x;
    }

    std::vector<uint8_t> RSA::to_bytes_fixed(cpp_int x) const
    {
        size_t k = (msb(n) + 8) / 8;
        std::vector<uint8_t> out(k, 0x00);
        for (size_t i = 0; i < k; i++)
        {
            out[k - 1 - i] = (uint8_t)(x & 0xFF);
            x >>= 8;
        }
        return out;
    }

    std::vector<uint8_t> RSA::asn1_len(size_t len)
    {
        if (len < 128)
            return {(uint8_t)len};
        std::vector<uint8_t> out;
        while (len > 0)
        {
            out.insert(out.begin(), len & 0xFF);
            len >>= 8;
        }
        out.insert(out.begin(), 0x80 | out.size());
        return out;
    }

    std::vector<uint8_t> RSA::asn1_int(const cpp_int &x)
    {
        std::vector<uint8_t> b;
        cpp_int v = x;
        while (v > 0)
        {
            b.insert(b.begin(), (uint8_t)(v & 0xFF));
            v >>= 8;
        }
        if (b.empty())
            b.push_back(0x00);
        if (b[0] & 0x80)
            b.insert(b.begin(), 0x00);
        std::vector<uint8_t> out = {0x02};
        auto len = asn1_len(b.size());
        out.insert(out.end(), len.begin(), len.end());
        out.insert(out.end(), b.begin(), b.end());
        return out;
    }

    std::vector<uint8_t> RSA::asn1_seq(const std::vector<uint8_t> &body)
    {
        std::vector<uint8_t> out = {0x30};
        auto len = asn1_len(body.size());
        out.insert(out.end(), len.begin(), len.end());
        out.insert(out.end(), body.begin(), body.end());
        return out;
    }

    std::string RSA::base64_encode_vec(const std::vector<uint8_t> &in)
    {
        std::string str(in.begin(), in.end());
        return base64_encode(str, str.size());
    }

    // --- PEM -> DER (robust): remove header/footer lines and join base64 lines ---
    std::vector<uint8_t> RSA::pem_to_der(const std::string &pem)
    {
        std::string b64;
        std::istringstream iss(pem);
        std::string line;
        while (std::getline(iss, line))
        {
            // skip header/footer and empty lines
            if (line.size() == 0)
                continue;
            if (line.rfind("-----", 0) == 0)
                continue;
            // append line stripped of whitespace
            for (char c : line)
                if (!std::isspace((unsigned char)c))
                    b64.push_back(c);
        }
        std::string decoded = base64_decode(b64, b64.size()); // baseN.h の関数を利用
        return std::vector<uint8_t>(decoded.begin(), decoded.end());
    }

    // --- ASN.1 length reader (advances pos) ---
    size_t RSA::read_asn1_len_from(const std::vector<uint8_t> &der, size_t &pos)
    {
        if (pos >= der.size())
            throw std::runtime_error("asn1: unexpected end");
        size_t len = der[pos++];
        if (len & 0x80)
        {
            int nbytes = len & 0x7F;
            if (nbytes == 0 || pos + nbytes > der.size())
                throw std::runtime_error("asn1: bad length");
            len = 0;
            for (int i = 0; i < nbytes; ++i)
            {
                len = (len << 8) | der[pos++];
            }
        }
        return len;
    }

    // --- ASN.1 INTEGER reader (robust) ---
    cpp_int RSA::read_asn1_int(const std::vector<uint8_t> &der, size_t &pos)
    {
        if (pos >= der.size() || der[pos] != 0x02)
            throw std::runtime_error("ASN1 int error");
        ++pos; // skip 0x02

        if (pos >= der.size())
            throw std::runtime_error("asn1: truncated");
        size_t len = der[pos++];

        if (len & 0x80)
        { // multi-byte length
            int nbytes = len & 0x7F;
            if (nbytes == 0 || pos + nbytes > der.size())
                throw std::runtime_error("asn1: bad length");
            len = 0;
            for (int i = 0; i < nbytes; ++i)
                len = (len << 8) | der[pos++];
        }

        if (pos + len > der.size())
            throw std::runtime_error("asn1: truncated value");

        // skip leading 0x00 padding used to ensure non-negative INTEGER
        size_t start = pos;
        while (start < pos + len && der[start] == 0x00)
            ++start;

        cpp_int out = 0;
        for (size_t i = start; i < pos + len; ++i)
        {
            out <<= 8;
            out |= der[i];
        }

        pos += len; // advance over the whole INTEGER field
        return out;
    }
}
