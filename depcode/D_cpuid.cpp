#include "D_cpuid.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>

#if defined(_WIN32)
#include <windows.h>
#include <intrin.h>
#else
#include <cpuid.h>
#endif

#include "D_sha.h"

namespace fasm::device
{
    static inline void cpuid_exec(uint32_t leaf, uint32_t subleaf,
                                  uint32_t &eax, uint32_t &ebx,
                                  uint32_t &ecx, uint32_t &edx)
    {
#if defined(_WIN32)
        int r[4];
        __cpuidex(r, leaf, subleaf);
        eax = r[0];
        ebx = r[1];
        ecx = r[2];
        edx = r[3];
#else
        __cpuid_count(leaf, subleaf, eax, ebx, ecx, edx);
#endif
    }

    static void get_cache_info(CPUInfo &info)
    {
        uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

        // --- get vendor string (to prefer AMD extended leaves) ---
        char vendor[13] = {0};
        cpuid_exec(0, 0, eax, ebx, ecx, edx);
        std::memcpy(vendor + 0, &ebx, 4);
        std::memcpy(vendor + 4, &edx, 4);
        std::memcpy(vendor + 8, &ecx, 4);
        std::string vendor_str = vendor;

        // --- query max basic and max extended ---
        uint32_t max_basic = eax; // from cpuid(0)
        cpuid_exec(0x80000000, 0, eax, ebx, ecx, edx);
        uint32_t max_ext = eax;

        // Clear previous values
        info.cache_l1 = info.cache_l2 = info.cache_l3 = 0;

        // --- 1) AMD-style extended leaves (prefer when available) ---
        if (vendor_str.find("AuthenticAMD") != std::string::npos && max_ext >= 0x80000005)
        {
            // L1: 0x80000005
            cpuid_exec(0x80000005, 0, eax, ebx, ecx, edx);
            // ECX[31:24] = L1 data size (KB), EDX[31:24] = L1 instruction size (KB)
            uint32_t l1d_kb = (ecx >> 24) & 0xFF;
            uint32_t l1i_kb = (edx >> 24) & 0xFF;
            info.cache_l1 = l1d_kb + l1i_kb;

            // L2/L3: 0x80000006 if present
            if (max_ext >= 0x80000006)
            {
                cpuid_exec(0x80000006, 0, eax, ebx, ecx, edx);
                // ECX[31:16] = L2 size (KB)
                info.cache_l2 = (ecx >> 16) & 0xFFFF;
                // EDX[31:18] = L3 size in 512KB units (AMD), multiply by 512
                uint32_t l3_units = (edx >> 18) & 0x3FFF;
                if (l3_units)
                    info.cache_l3 = l3_units * 512;
                else
                {
                    // some AMDs put L3 in ECX/EDX differently; fallback later
                }
            }
        }

        // --- 2) Intel-style deterministic cache parameters (leaf 4) ---
        if (max_basic >= 4)
        {
            for (uint32_t i = 0;; ++i)
            {
                cpuid_exec(4, i, eax, ebx, ecx, edx);
                uint32_t type = eax & 0x1F;
                if (type == 0)
                    break; // no more caches

                uint32_t level = (eax >> 5) & 0x7;
                uint32_t line_size = (ebx & 0xFFF) + 1;
                uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
                uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
                uint32_t sets = ecx + 1;

                uint64_t bytes = (uint64_t)ways * partitions * line_size * sets;
                uint32_t size_kb = (uint32_t)(bytes / 1024);

                if (level == 1)
                    info.cache_l1 += size_kb;
                else if (level == 2)
                    info.cache_l2 += size_kb;
                else if (level == 3)
                    info.cache_l3 += size_kb;
                // continue to next index
            }
        }

        // --- 3) Fallback: extended leaf 0x80000006 for non-AMD or when L1 missing ---
        // Some environments (VMs) hide leaf 4 or AMD L1; try 0x80000006 anyway if present.
        if (info.cache_l1 == 0 && max_ext >= 0x80000005)
        {
            cpuid_exec(0x80000005, 0, eax, ebx, ecx, edx);
            uint32_t l1d_kb = (ecx >> 24) & 0xFF;
            uint32_t l1i_kb = (edx >> 24) & 0xFF;
            if ((l1d_kb + l1i_kb) != 0)
                info.cache_l1 = l1d_kb + l1i_kb;
        }

        if ((info.cache_l2 == 0 || info.cache_l3 == 0) && max_ext >= 0x80000006)
        {
            cpuid_exec(0x80000006, 0, eax, ebx, ecx, edx);
            if (info.cache_l2 == 0)
                info.cache_l2 = (ecx >> 16) & 0xFFFF;
            if (info.cache_l3 == 0)
            {
                uint32_t l3_units = (edx >> 18) & 0x3FFF;
                if (l3_units)
                    info.cache_l3 = l3_units * 512;
                else
                {
                    // Some implementations may give L3 in ECX differently; try best-effort:
                    // Old AMD docs: ECX contains L2, EDX contains L3 units — handled above.
                }
            }
        }

        // --- done. info.cache_l1/2/3 are in KB (0 means not determined) ---
    }

    static void get_core_info(CPUInfo &info)
    {
        uint32_t eax, ebx, ecx, edx;

        cpuid_exec(1, 0, eax, ebx, ecx, edx);
        info.logical_threads = (ebx >> 16) & 0xFF;

        // AMD correct method
        cpuid_exec(0x80000008, 0, eax, ebx, ecx, edx);

        uint32_t cores = (ecx & 0xFF) + 1;
        info.physical_cores = cores;
    }

    CPUInfo get_cpu_info()
    {
        CPUInfo info;
        uint32_t eax, ebx, ecx, edx;

        // ===== vendor =====
        char vendor[13]{};
        cpuid_exec(0, 0, eax, ebx, ecx, edx);
        std::memcpy(vendor + 0, &ebx, 4);
        std::memcpy(vendor + 4, &edx, 4);
        std::memcpy(vendor + 8, &ecx, 4);
        info.vendor = vendor;

        // ===== brand =====
        char brand[49]{};
        uint32_t max_ext;
        cpuid_exec(0x80000000, 0, max_ext, ebx, ecx, edx);

        if (max_ext >= 0x80000004)
        {
            uint32_t *b = reinterpret_cast<uint32_t *>(brand);
            for (uint32_t i = 0; i < 3; ++i)
            {
                cpuid_exec(0x80000002 + i, 0,
                           b[i * 4 + 0],
                           b[i * 4 + 1],
                           b[i * 4 + 2],
                           b[i * 4 + 3]);
            }
        }
        info.brand = brand;

        // ===== feature =====
        cpuid_exec(1, 0, eax, ebx, ecx, edx);
        info.logical_threads = (ebx >> 16) & 0xFF;
        info.feature_ecx = ecx;
        info.feature_edx = edx;

        cpuid_exec(7, 0, eax, ebx, ecx, edx);
        info.feature7_ebx = ebx;

        get_core_info(info);
        get_cache_info(info);

        return info;
    }

#if defined(_WIN32)

    static std::string smbios_string(uint8_t *str, int index)
    {
        if (index == 0)
            return {};
        int i = 1;
        while (*str)
        {
            if (i == index)
                return std::string((char *)str);
            str += strlen((char *)str) + 1;
            i++;
        }
        return {};
    }

    static SMBIOSInfo get_smbios_windows()
    {
        SMBIOSInfo out;

        DWORD size = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);
        if (!size)
            return out;

        std::vector<uint8_t> buf(size);
        if (!GetSystemFirmwareTable('RSMB', 0, buf.data(), size))
            return out;

        struct RawSMBIOS
        {
            uint8_t used;
            uint8_t major;
            uint8_t minor;
            uint8_t rev;
            uint32_t length;
            uint8_t data[1];
        };

        RawSMBIOS *raw = (RawSMBIOS *)buf.data();
        uint8_t *data = raw->data;
        size_t len = raw->length;

        size_t i = 0;

        while (i < len)
        {
            uint8_t type = data[i];
            uint8_t slen = data[i + 1];

            uint8_t *strings = &data[i + slen];

            switch (type)
            {
            case 0: // BIOS
                out.bios_vendor = smbios_string(strings, data[i + 4]);
                out.bios_version = smbios_string(strings, data[i + 5]);
                break;

            case 1: // UUID
            {
                uint8_t *u = &data[i + 8];
                char uuid[37];
                snprintf(uuid, sizeof(uuid),
                         "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-"
                         "%02X%02X%02X%02X%02X%02X",
                         u[3], u[2], u[1], u[0],
                         u[5], u[4],
                         u[7], u[6],
                         u[8], u[9],
                         u[10], u[11], u[12], u[13], u[14], u[15]);
                out.uuid = uuid;
                break;
            }

            case 2: // motherboard
                out.board_serial = smbios_string(strings, data[i + 7]);
                break;

            case 3: // chassis
                out.chassis_serial = smbios_string(strings, data[i + 7]);
                break;
            }

            i += slen;

            while (i < len - 1)
            {
                if (data[i] == 0 && data[i + 1] == 0)
                {
                    i += 2;
                    break;
                }
                i++;
            }
        }

        return out;
    }

#endif

#if defined(__linux__)

    static std::string read_file(const std::string &path)
    {
        std::ifstream f(path);
        if (!f)
            return {};
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    static SMBIOSInfo get_smbios_linux()
    {
        SMBIOSInfo out;

        out.uuid = read_file("/sys/class/dmi/id/product_uuid");
        out.bios_vendor = read_file("/sys/class/dmi/id/bios_vendor");
        out.bios_version = read_file("/sys/class/dmi/id/bios_version");
        out.board_serial = read_file("/sys/class/dmi/id/board_serial");
        out.chassis_serial = read_file("/sys/class/dmi/id/chassis_serial");

        return out;
    }

#endif

    SMBIOSInfo get_smbios()
    {
#if defined(_WIN32)
        return get_smbios_windows();
#elif defined(__linux__)
        return get_smbios_linux();
#else
        return {};
#endif
    }

    // ===== fingerprint =====
    std::string generate_device_fingerprint()
    {
        CPUInfo c = get_cpu_info();

        std::string data;

        // stable hardware identity
        data += c.vendor;
        data += c.brand;
        data += std::to_string(c.logical_threads);
        data += std::to_string(c.physical_cores);
        data += std::to_string(c.feature_ecx);
        data += std::to_string(c.feature_edx);
        data += std::to_string(c.feature7_ebx);
        data += std::to_string(c.cache_l1);
        data += std::to_string(c.cache_l2);
        data += std::to_string(c.cache_l3);

        // CPUID serial-like leaves
        uint32_t eax, ebx, ecx, edx;

        // extended CPU info
        cpuid_exec(0x80000001, 0, eax, ebx, ecx, edx);
        data += std::to_string(ecx);
        data += std::to_string(edx);

        // SMBIOSInfo
        SMBIOSInfo smbios = get_smbios();
        data += smbios.uuid;
        data += smbios.bios_vendor;
        data += smbios.bios_version;
        data += smbios.board_serial;
        data += smbios.chassis_serial;

        return hash::SHA256::str256(data);
    }
}
