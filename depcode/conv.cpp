#include "conv.h"

std::vector<uint8_t> str2vec(const std::string &s)
{
    return std::vector<uint8_t>(s.begin(), s.end());
}
std::string vec2str(const std::vector<uint8_t> &v)
{
    return std::string(v.begin(), v.end());
}
