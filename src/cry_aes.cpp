#include "cryptaes.h"
#include "rsa.h"
#include "aes.h"
#include "conv.h"

#include <unordered_map>
#include <vector>
#include <random>

namespace unet::cry
{
    enum message_type : int32_t
    {
        rData = 0,
        cLose = 1,
        cOnnectReq = 2,
        aCceptReq = 3,
    };

    using aesKey = std::vector<uint8_t>;

    static std::unordered_map<int, aesKey> aeskeys;
    static std::unordered_map<int, std::string> sockBuffers;
    static std::unordered_map<int, cryptASM::RSA> rsa_ctx;

    static bool recv_all(int s, char *buf, int len, int flags)
    {
        int recvd = 0;
        while (recvd < len)
        {
            int r = recv(s, buf + recvd, len - recvd, flags);
            if (r <= 0)
                return false;
            recvd += r;
        }
        return true;
    }

    static aesKey gen_aes_key()
    {
        aesKey k(32);
        std::random_device rd;
        for (auto &b : k)
            b = (uint8_t)rd();
        return k;
    }

    int send_crypt(int s, const char *buf, int len, int flags)
    {
        std::string datar(buf, len);
        cryptASM::AES256 aes(aeskeys[s]);
        std::string datac = vec2str(aes.encrypt(str2vec(datar)));

        size_t datac_size = datac.size();
        int32_t data_type = (int32_t)rData;

        if (send(s, (char *)&data_type, sizeof(data_type), flags) <= 0)
            return -1;
        if (send(s, (char *)&datac_size, sizeof(datac_size), flags) <= 0)
            return -1;
        return send(s, datac.c_str(), datac.size(), flags);
    }

    int recv_crypt(int s, char *buf, int len, int flags)
    {
        if (sockBuffers[s].empty())
        {
            int32_t type;

            if (!recv_all(s, (char *)&type, sizeof(type), flags))
                return -1;

            if (type == cLose)
                return 0;

            size_t enc_size;
            if (!recv_all(s, (char *)&enc_size, sizeof(enc_size), flags))
                return -1;

            std::string enc(enc_size, 0);

            if (!recv_all(s, enc.data(), (int)enc_size, flags))
                return -1;

            cryptASM::AES256 aes(aeskeys[s]);
            std::string dec = vec2str(aes.decrypt(str2vec(enc)));

            sockBuffers[s] += dec;
        }

        std::string &buffer = sockBuffers[s];
        int copy_len = (int)std::min((size_t)len, buffer.size());

        memcpy(buf, buffer.data(), copy_len);
        buffer.erase(0, copy_len);

        return copy_len;
    }

    /*
        ======================
        RSA HANDSHAKE
        ======================
    */

    int connect_crypt(int s, const struct sockaddr *name, int namelen)
    {
        int r = connect(s, name, namelen);
        if (r < 0)
            return r;

        sockBuffers[s] = "";

        /* 1. RSA key generate */
        cryptASM::RSA rsa;
        rsa.generate_keys();
        rsa_ctx[s] = rsa;

        std::string pub = rsa.export_public_key_pem();
        size_t pub_size = pub.size();

        /* 2. send connect request + public key */
        int32_t type = (int32_t)cOnnectReq;
        if (send(s, (char *)&type, sizeof(type), 0) <= 0)
            return -1;

        if (send(s, (char *)&pub_size, sizeof(pub_size), 0) <= 0)
            return -1;

        if (send(s, pub.data(), pub_size, 0) <= 0)
            return -1;

        /* 3. receive encrypted AES key */
        int32_t atype;
        if (!recv_all(s, (char *)&atype, sizeof(atype), 0))
            return -1;

        if (atype != aCceptReq)
            return -1;

        size_t key_size;
        if (!recv_all(s, (char *)&key_size, sizeof(key_size), 0))
            return -1;

        std::vector<uint8_t> enc(key_size);
        if (!recv_all(s, (char *)enc.data(), (int)key_size, 0))
            return -1;

        auto aes = rsa.decrypt(enc);
        aeskeys[s] = aes;

        rsa_ctx.erase(s);

        return 0;
    }

    int accept_crypt(int s, struct sockaddr *addr, int *addrlen)
    {
        int ns = accept(s, addr, addrlen);
        if (ns < 0)
            return ns;

        sockBuffers[ns] = "";

        /* 1. receive connect request */
        int32_t type;
        if (!recv_all(ns, (char *)&type, sizeof(type), 0))
            return -1;

        if (type != cOnnectReq)
            return -1;

        /* 2. receive client public key */
        size_t pub_size;
        if (!recv_all(ns, (char *)&pub_size, sizeof(pub_size), 0))
            return -1;

        std::string pub(pub_size, 0);
        if (!recv_all(ns, pub.data(), (int)pub_size, 0))
            return -1;

        cryptASM::RSA rsa_pub;
        rsa_pub.import_public_key_pem(pub);

        /* 3. generate AES session key */
        aesKey aes = gen_aes_key();
        aeskeys[ns] = aes;

        /* 4. encrypt AES key */
        auto enc = rsa_pub.encrypt(aes);
        size_t enc_size = enc.size();

        /* 5. send accept */
        int32_t atype = (int32_t)aCceptReq;
        if (send(ns, (char *)&atype, sizeof(atype), 0) <= 0)
            return -1;

        if (send(ns, (char *)&enc_size, sizeof(enc_size), 0) <= 0)
            return -1;

        if (send(ns, (char *)enc.data(), enc_size, 0) <= 0)
            return -1;

        return ns;
    }

    int close_crypt(int s)
    {
        int32_t type = (int32_t)cLose;
        send(s, (char *)&type, sizeof(type), 0);

        aeskeys.erase(s);
        sockBuffers.erase(s);
        rsa_ctx.erase(s);

        return close(s);
    }

    int shutdown_crypt(int s, int how)
    {
        /* AES セッションが存在する場合のみ通知 */
        if (aeskeys.find(s) != aeskeys.end())
        {
            /* 送信側 shutdown の時だけ close 通知 */
            if (how == SD_SEND || how == SD_BOTH)
            {
                int32_t type = (int32_t)cLose;
                send(s, (char *)&type, sizeof(type), 0);
            }

            /* 受信停止 or 両方停止ならバッファ破棄 */
            if (how == SD_RECEIVE || how == SD_BOTH)
            {
                sockBuffers.erase(s);
            }

            /* 両方向停止なら鍵も破棄 */
            if (how == SD_BOTH)
            {
                aeskeys.erase(s);
                rsa_ctx.erase(s);
            }
        }

        return shutdown(s, how);
    }
}
