#include "md5.h"

#include "baseN.h"

#include "md5c.h"
#include <cstring>
#include <sstream>
#include <iomanip>

void MD5::md5_gen(const std::string &str, int32_t len)
{
    MD5_CTX ctx;
    MD5Init(&ctx);
    MD5Update(&ctx, (unsigned char *)str.c_str(), len);
    MD5Final(res, &ctx);
}

void MD5::md5_hex(const char *data, unsigned char res[], int32_t len)
{
    if (len == -1)
        len = strlen(data);
    MD5_CTX ctx;
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
    return base16_encode(hex);
}
