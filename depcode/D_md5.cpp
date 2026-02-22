#include "D_md5.h"

#include "D_baseN.h"

#include "D_md5c.h"
#include <cstring>
#include <sstream>
#include <iomanip>

namespace fasm::hash
{
    void MD5::md5_gen(const std::string &str, int32_t len)
    {
        md5::MD5_CTX ctx;
        MD5Init(&ctx);
        MD5Update(&ctx, (unsigned char *)str.c_str(), len);
        MD5Final(res, &ctx);
    }

    void MD5::md5_hex(const char *data, unsigned char res[], int32_t len)
    {
        if (len == -1)
            len = strlen(data);
        md5::MD5_CTX ctx;
        MD5Init(&ctx);
        MD5Update(&ctx, (unsigned char *)data, len);
        MD5Final(res, &ctx);
    }

    std::string MD5::hash(const std::string &str, int32_t len)
    {
        if (len == -1)
            len = str.size();
        MD5 ctx;
        ctx.md5_gen(str, len);
        return std::string((char *)ctx.res, 16);
    }

    std::string MD5::str(const std::string &str, int32_t len)
    {
        if (len == -1)
            len = str.size();
        std::string hex = hash(str, len);
        return enc::base16_encode(hex);
    }
}
