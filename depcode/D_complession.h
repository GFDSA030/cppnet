#pragma once
#include <string>

#if __has_include("zlib.h")
namespace fasm::inline data
{
    std::string complession(const std::string &data);

    std::string decomplession(const std::string &data);
}
#endif
