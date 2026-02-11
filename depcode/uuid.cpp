#include "uuid.h"
#include "baseN.h"

#include <iostream>
#include <chrono>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <random>

#ifdef _WIN32
#include <iphlpapi.h>
#else
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <cstring>
#endif

static std::random_device rd;

// Get Mac Address
bool GetMacAddress(uint8_t mac[6])
{
#ifdef _WIN32
    IP_ADAPTER_INFO AdapterInfo[16]; // 最大16個のアダプタ情報を格納する配列
    DWORD dwBufLen = sizeof(AdapterInfo);
    DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
    if (dwStatus != ERROR_SUCCESS)
    {
        return false; // 失敗
    }

    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
    if (pAdapterInfo == nullptr)
    {
        return false; // 失敗
    }

    // 最初のアダプタのMACアドレスを取得
    for (int i = 0; i < 6; ++i)
    {
        mac[i] = pAdapterInfo->Address[i];
    }
    return true; // 成功
#else
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1)
    {
        return false; // 失敗
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
            continue;

        if (ifa->ifa_addr->sa_family == AF_PACKET)
        {
            struct sockaddr_ll *s = (struct sockaddr_ll *)ifa->ifa_addr;
            if (s->sll_halen == 6) // MACアドレスの長さを確認
            {
                memcpy(mac, s->sll_addr, 6);
                freeifaddrs(ifaddr);
                return true; // 成功
            }
        }
    }

    freeifaddrs(ifaddr);
    return false; // 失敗
#endif
}

uuidshort uuid::encode() const
{
    uuidshort us = {};
    std::string res = base32_encode((char *)this, sizeof(*this));
    memcpy(&us.data, res.c_str(), res.size());
    us.data[uuidsdatalen] = '\0';
    return us;
}

uuid uuidshort::decode() const
{
    uuid u = {};
    std::string res = base32_decode((char *)this, sizeof(*this));
    memcpy((void *)&u, res.c_str(), res.size());

    return u;
}

uuid uuid::Print() const
{
    printf("%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\n", d1, d2, d3, d4, d5[0], d5[1], d5[2], d5[3], d5[4], d5[5]);
    return *this;
}
std::string uuid::str() const
{
    char buf[37] = {};
    sprintf(buf, "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x", d1, d2, d3, d4, d5[0], d5[1], d5[2], d5[3], d5[4], d5[5]);
    return std::string(buf);
}
uuid uuid::SetNewID(uint64_t d0, uint64_t d1)
{
    uuid ret;
    ret.data0 = d0;
    ret.data1 = d1;
    return ret;
}
uuidshort uuidshort::SetNewID(const char *d)
{
    uuidshort ret;
    memcpy(ret.data, d, sizeof(data));
    return ret;
}
uuid uuid::SetID(uint64_t d0, uint64_t d1)
{
    this->data0 = d0;
    this->data1 = d1;
    return *this;
}
uuidshort uuidshort::SetID(const char *d)
{
    memcpy(this->data, d, sizeof(data));
    return *this;
}
uuidshort uuidshort::Print() const
{
    for (size_t i = 0; i < sizeof(data); ++i)
    {
        printf("%c", data[i]);
    }
    printf("\n");
    return *this;
}
std::string uuidshort::str() const
{
    return std::string(data);
}
uuid uuid::GenV1()
{
    uuid id = {};
    uint16_t rdval = (uint16_t)rd();
    std::timespec tp = {};
    std::timespec_get(&tp, TIME_UTC);
    std::uint64_t time = (std::uint64_t)tp.tv_sec * 10000000 + (std::uint64_t)tp.tv_nsec / 100;
    time += 0x01B21DD213814000ULL; // 1582-10-15 00:00:00 UTCからの100ナノ秒単位の経過時間

    id.d1 = (time & 0xFFFFFFFF);
    id.d2 = (time >> 32) & 0xFFFF;
    id.d3 = (0x1 << 12) | ((time >> 48) & 0x0FFF);
    // id.d3_0 = 1; // バージョン1
    // id.d3_1 = (time >> 48) & 0x0FFF;
    id.d4 = rdval;                     // クロックシーケンス
    id.d4 = (id.d4 & 0x3FFF) | 0x8000; // バリアント（RFC 4122）

    GetMacAddress(id.d5);

    return id;
}
uuid uuid::GenV4()
{
    uuid id = {};
    uint32_t rdval = (uint32_t)rd();
    id.d1 = rdval;
    rdval = (uint32_t)rd();
    id.d2 = rdval & 0xFFFF;
    rdval = (uint32_t)rd();
    id.d3 = (0x4 << 12) | (rdval & 0x0FFF);
    // id.d3_0 = 4; // バージョン4
    // id.d3_1 = (rdval & 0x0FFF);
    rdval = (uint32_t)rd();
    id.d4 = (rdval) & 0xFFFF;
    id.d4 = (id.d4 & 0x3FFF) | 0x8000; // バリアント（RFC 4122）
    rdval = (uint32_t)rd();
    id.d5[0] = (rdval) & 0xFF;
    rdval = (uint32_t)rd();
    id.d5[1] = (rdval) & 0xFF;
    rdval = (uint32_t)rd();
    id.d5[2] = (rdval) & 0xFF;
    rdval = (uint32_t)rd();
    id.d5[3] = (rdval) & 0xFF;
    rdval = (uint32_t)rd();
    id.d5[4] = (rdval) & 0xFF;
    rdval = (uint32_t)rd();
    id.d5[5] = (rdval) & 0xFF;

    return id;
}
uuid uuid::GenV7()
{
    uuid id = {};
    std::timespec tp = {};
    std::timespec_get(&tp, TIME_UTC);
    uint16_t rdval = 0;
    std::uint64_t time = (std::uint64_t)tp.tv_sec * 1000 + (std::uint64_t)tp.tv_nsec / 1000000;
    id.d1 = (time >> 16) & 0xFFFFFFFF;
    id.d2 = time & 0xFFFF;
    rdval = (uint16_t)rd();
    id.d3 = (0x7 << 12) | (rdval & 0x0FFF);
    // id.d3_0 = 7; // バージョン7
    // id.d3_1 = rdval & 0x0FFF;
    rdval = (uint16_t)rd();
    id.d4 = rdval;                     // クロックシーケンス
    id.d4 = (id.d4 & 0x3FFF) | 0x8000; // バリアント（RFC 4122）
    rdval = (uint16_t)rd();
    id.d5[0] = (rdval >> 8) & 0xFF;
    rdval = (uint32_t)rd();
    id.d5[1] = (rdval) & 0xFF;
    rdval = (uint16_t)rd();
    id.d5[2] = (rdval >> 8) & 0xFF;
    rdval = (uint32_t)rd();
    id.d5[3] = (rdval) & 0xFF;
    rdval = (uint16_t)rd();
    id.d5[4] = (rdval >> 8) & 0xFF;
    rdval = (uint32_t)rd();
    id.d5[5] = (rdval) & 0xFF;
    return id;
}
