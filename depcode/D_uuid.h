#pragma once
#include <cstdint>
#include <cstring>
#include <string>
namespace fasm::inline id
{
    class uuidshort;
    class uuid;
    class uuid
    {
    public:
        union
        {
            uint64_t data0 = 0;
            struct
            {
                uint32_t d1; // タイムスタンプの下位32ビット
                uint16_t d2; // タイムスタンプの中位16ビット
                uint16_t d3; // タイムスタンプの上位12ビットとバージョン
            };
        };
        union
        {
            uint64_t data1 = 0;
            struct
            {
                uint16_t d4;   // クロックシーケンス
                uint8_t d5[6]; // MACアドレス
            };
        };

        uuid() {}
        uuid(const uuid &other)
        {
            data0 = other.data0;
            data1 = other.data1;
        }
        uuidshort encode() const;
        uuid Print() const;
        std::string str() const;
        static uuid SetNewID(uint64_t d0, uint64_t d1);
        uuid SetID(uint64_t d0, uint64_t d1);

        static uuid GenV1();
        static uuid GenV4();
        static uuid GenV7();
    };

    static constexpr size_t databit = 5;
    static constexpr size_t uuidsdatalen = (sizeof(uuid) * 8 + databit - 1) / databit;
    class uuidshort
    {
    public:
        uuidshort() {}
        uuidshort(const uuidshort &other)
        {
            memcpy(data, other.data, sizeof(data));
        }

        char data[uuidsdatalen + 1] = {};
        uuid decode() const;
        uuidshort Print() const;
        std::string str() const;
        static uuidshort SetNewID(const char *d);
        uuidshort SetID(const char *d);
    };
}
