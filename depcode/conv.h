#pragma once
#include <vector>
#include <string>
#include <cstdint>

std::vector<uint8_t> str2vec(const std::string &s);
std::string vec2str(const std::vector<uint8_t> &v);
