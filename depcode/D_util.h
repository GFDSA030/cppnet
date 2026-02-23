#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace fasm::util
{
    std::vector<uint8_t> str2vec(const std::string &s);
    std::string vec2str(const std::vector<uint8_t> &v);

    /*
    https://kryozahiro.hateblo.jp/entry/20080809/1218295912
    */
    // ワイド文字列からマルチバイト文字列
    // ロケール依存
    void narrow(const std::wstring &src, std::string &dest);

    // マルチバイト文字列からワイド文字列
    // ロケール依存
    void widen(const std::string &src, std::wstring &dest);
}
