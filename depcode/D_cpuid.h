#pragma once
#include <string>
#include <cstdint>

// ===== CPUID =====
struct CPUInfo
{
    std::string vendor;
    std::string brand;

    uint32_t logical_threads = 0;
    uint32_t physical_cores = 0;

    uint32_t feature_ecx = 0;
    uint32_t feature_edx = 0;
    uint32_t feature7_ebx = 0;

    uint32_t cache_l1 = 0;
    uint32_t cache_l2 = 0;
    uint32_t cache_l3 = 0;
};

struct SMBIOSInfo
{
    std::string uuid;
    std::string bios_vendor;
    std::string bios_version;
    std::string board_serial;
    std::string chassis_serial;
};

// ===== CPUinfo =====
CPUInfo get_cpu_info();

// ===== get_smbios =====
SMBIOSInfo get_smbios();

// ===== fingerprint =====
std::string generate_device_fingerprint();
