#pragma once

/*
https://kryozahiro.hateblo.jp/entry/20080809/1218295912
*/

#include <cstdlib>
#include <string>

// ワイド文字列からマルチバイト文字列
// ロケール依存
void narrow(const std::wstring &src, std::string &dest)
{
    char *mbs = new char[src.length() * MB_CUR_MAX + 1];
    wcstombs(mbs, src.c_str(), src.length() * MB_CUR_MAX + 1);
    dest = mbs;
    delete[] mbs;
}

// マルチバイト文字列からワイド文字列
// ロケール依存
void widen(const std::string &src, std::wstring &dest)
{
    wchar_t *wcs = new wchar_t[src.length() + 1];
    mbstowcs(wcs, src.c_str(), src.length() + 1);
    dest = wcs;
    delete[] wcs;
}
