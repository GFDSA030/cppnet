#pragma once
#include <cstdint>
#include <string>

static constexpr char base16_table[] = "0123456789abcdef";
// static constexpr char base16_table[] = "0123456789ABCDEF";
static constexpr char base32_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
// static constexpr char base32_table[] = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ";
// static constexpr char base32_table[] = "ABCDEFGHKLMNPQRSTUVWXYZabcdefghk";
static constexpr char base58_table[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static constexpr char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
// static constexpr char base64_table[] = "KYFOB89-zsdXRAMGPaJihN3evbW4ZVmHIQ_f56701nopqrgjkcxy2STUlwLCDEtu";
static constexpr char base85_table[] = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstu";
static constexpr char base91_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
                                       "!#$%&()*+,./:;<=>?@[]^_`{|}~\"";
static constexpr char base122_table[] = "";
static constexpr char base32768_table[] = "";

char base64_encode_table(const int8_t d);
uint8_t base64_decode_table(const char d);
std::string base64_encode(const std::string &src, int32_t len = -1); // 元データ　元データサイズ
std::string base64_decode(const std::string &src, int32_t len = -1); // エンコード済みデータ　エンコード済みデータサイズ

char base32_encode_table(const int8_t d);
uint8_t base32_decode_table(const char d);
std::string base32_encode(const std::string &src, int32_t len = -1); // 元データ　元データサイズ
std::string base32_decode(const std::string &src, int32_t len = -1); // エンコード済みデータ　エンコード済みデータサイズ

char base16_encode_table(const int8_t d);
uint8_t base16_decode_table(const char d);
std::string base16_encode(const std::string &src, int32_t len = -1); // 元データ　元データサイズ
std::string base16_decode(const std::string &src, int32_t len = -1); // エンコード済みデータ　エンコード済みデータサイズ

char baseN_encode_table(const size_t d, const char *table);
uint8_t baseN_decode_table(const char d, const char *table);
std::string baseN_encode(const std::string &src, int32_t len = -1, const char *table = base64_table); // 元データ　元データサイズ
std::string baseN_decode(const std::string &src, int32_t len = -1, const char *table = base64_table); // エンコード済みデータ　エンコード済みデータサイズ
std::string baseN_encode(const std::string &src, const char *table = base64_table, int32_t len = -1); // 元データ　元データサイズ
std::string baseN_decode(const std::string &src, const char *table = base64_table, int32_t len = -1); // エンコード済みデータ　エンコード済みデータサイズ
