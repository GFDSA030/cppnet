#pragma once
#include <string>
#include <cstdint>
namespace fasm::inline hash
{
    class MD5
    {
    private:
        unsigned char res[16] = {};
        void md5_gen(const std::string &str, int32_t len);

    public:
        static void md5_hex(const char *data, unsigned char res[], int32_t len = -1);

        static std::string hash(const std::string &str, int32_t len = -1);

        static std::string str(const std::string &str, int32_t len = -1);
    };
}
