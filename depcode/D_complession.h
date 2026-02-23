#pragma once
#if __has_include("zlib.h")
#include <string>

namespace fasm::data
{
    std::string complession(const std::string &data);

    std::string decomplession(const std::string &data);
}
#endif
