#pragma once

#include <stdint.h>
#include <string>

// Reference: https://docs.microsoft.com/en-us/windows/win32/debug/pe-format

namespace COFF
{
    // Machine types
    static constexpr uint16_t IMAGE_FILE_MACHINE_UNKNOWN    = 0;
    static constexpr uint16_t IMAGE_FILE_MACHINE_AM33       = 0x1d3;
    static constexpr uint16_t IMAGE_FILE_MACHINE_AMD64      = 0x8664;
    static constexpr uint16_t IMAGE_FILE_MACHINE_ARM        = 0x1c0;
    static constexpr uint16_t IMAGE_FILE_MACHINE_ARM64      = 0xaa64;
    static constexpr uint16_t IMAGE_FILE_MACHINE_ARMNT      = 0x1c4;
    static constexpr uint16_t IMAGE_FILE_MACHINE_EBC        = 0xebc;
    static constexpr uint16_t IMAGE_FILE_MACHINE_I386       = 0x14c;
    static constexpr uint16_t IMAGE_FILE_MACHINE_IA64       = 0x200;
    static constexpr uint16_t IMAGE_FILE_MACHINE_M32R       = 0x9041;
    static constexpr uint16_t IMAGE_FILE_MACHINE_MIPS16     = 0x266;
    static constexpr uint16_t IMAGE_FILE_MACHINE_MIPSFPU    = 0x366;
    static constexpr uint16_t IMAGE_FILE_MACHINE_MIPSFPU16  = 0x466;
    static constexpr uint16_t IMAGE_FILE_MACHINE_POWERPC    = 0x1f0;
    static constexpr uint16_t IMAGE_FILE_MACHINE_POWERPCFP  = 0x1f1;
    static constexpr uint16_t IMAGE_FILE_MACHINE_R4000      = 0x166;
    static constexpr uint16_t IMAGE_FILE_MACHINE_RISCV32    = 0x5032;
    static constexpr uint16_t IMAGE_FILE_MACHINE_RISCV64    = 0x5064;
    static constexpr uint16_t IMAGE_FILE_MACHINE_RISCV128   = 0x5128;
    static constexpr uint16_t IMAGE_FILE_MACHINE_SH3        = 0x1a2;
    static constexpr uint16_t IMAGE_FILE_MACHINE_SH3DSP     = 0x1a3;
    static constexpr uint16_t IMAGE_FILE_MACHINE_SH4        = 0x1a6;
    static constexpr uint16_t IMAGE_FILE_MACHINE_SH5        = 0x1a8;
    static constexpr uint16_t IMAGE_FILE_MACHINE_THUMB      = 0x1c2;
    static constexpr uint16_t IMAGE_FILE_MACHINE_WCEMIPSV2  = 0x169;

    // Characteristics
    static constexpr uint16_t IMAGE_FILE_RELOCS_STRIPPED            = 0x0001;
    static constexpr uint16_t IMAGE_FILE_EXECUTABLE_IMAGE           = 0x0002;
    static constexpr uint16_t IMAGE_FILE_LINE_NUMS_STRIPPED         = 0x0004;
    static constexpr uint16_t IMAGE_FILE_LOCAL_SYMS_STRIPPED        = 0x0008;
    static constexpr uint16_t IMAGE_FILE_AGGRESSIVE_WS_TRIM         = 0x0010;
    static constexpr uint16_t IMAGE_FILE_LARGE_ADDRESS_AWARE        = 0x0020;
    static constexpr uint16_t IMAGE_FILE_BYTES_REVERSED_LO          = 0x0080;
    static constexpr uint16_t IMAGE_FILE_32BIT_MACHINE              = 0x0100;
    static constexpr uint16_t IMAGE_FILE_DEBUG_STRIPPED             = 0x0200;
    static constexpr uint16_t IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP    = 0x0400;
    static constexpr uint16_t IMAGE_FILE_NET_RUN_FROM_SWAP          = 0x0800;
    static constexpr uint16_t IMAGE_FILE_SYSTEM                     = 0x1000;
    static constexpr uint16_t IMAGE_FILE_DLL                        = 0x2000;
    static constexpr uint16_t IMAGE_FILE_UP_SYSTEM_ONLY             = 0x4000;
    static constexpr uint16_t IMAGE_FILE_BYTES_REVERSED_HI          = 0x8000;

    static inline
    std::string machine_type_to_string(const uint16_t machine)
    {
        switch (machine)
        {
            case IMAGE_FILE_MACHINE_UNKNOWN:    return "IMAGE_FILE_MACHINE_UNKNOWN";
            case IMAGE_FILE_MACHINE_AM33:       return "IMAGE_FILE_MACHINE_AM33";
            case IMAGE_FILE_MACHINE_AMD64:      return "IMAGE_FILE_MACHINE_AMD64";
            case IMAGE_FILE_MACHINE_ARM:        return "IMAGE_FILE_MACHINE_ARM";
            case IMAGE_FILE_MACHINE_ARM64:      return "IMAGE_FILE_MACHINE_ARM64";
            case IMAGE_FILE_MACHINE_ARMNT:      return "IMAGE_FILE_MACHINE_ARMNT";
            case IMAGE_FILE_MACHINE_EBC:        return "IMAGE_FILE_MACHINE_EBC";
            case IMAGE_FILE_MACHINE_I386:       return "IMAGE_FILE_MACHINE_I386";
            case IMAGE_FILE_MACHINE_IA64:       return "IMAGE_FILE_MACHINE_IA64";
            case IMAGE_FILE_MACHINE_M32R:       return "IMAGE_FILE_MACHINE_M32R";
            case IMAGE_FILE_MACHINE_MIPS16:     return "IMAGE_FILE_MACHINE_MIPS16";
            case IMAGE_FILE_MACHINE_MIPSFPU:    return "IMAGE_FILE_MACHINE_MIPSFPU";
            case IMAGE_FILE_MACHINE_MIPSFPU16:  return "IMAGE_FILE_MACHINE_MIPSFPU16";
            case IMAGE_FILE_MACHINE_POWERPC:    return "IMAGE_FILE_MACHINE_POWERPC";
            case IMAGE_FILE_MACHINE_POWERPCFP:  return "IMAGE_FILE_MACHINE_POWERPCFP";
            case IMAGE_FILE_MACHINE_R4000:      return "IMAGE_FILE_MACHINE_R4000";
            case IMAGE_FILE_MACHINE_RISCV32:    return "IMAGE_FILE_MACHINE_RISCV32";
            case IMAGE_FILE_MACHINE_RISCV64:    return "IMAGE_FILE_MACHINE_RISCV64";
            case IMAGE_FILE_MACHINE_RISCV128:   return "IMAGE_FILE_MACHINE_RISCV128";
            case IMAGE_FILE_MACHINE_SH3:        return "IMAGE_FILE_MACHINE_SH3";
            case IMAGE_FILE_MACHINE_SH3DSP:     return "IMAGE_FILE_MACHINE_SH3DSP";
            case IMAGE_FILE_MACHINE_SH4:        return "IMAGE_FILE_MACHINE_SH4";
            case IMAGE_FILE_MACHINE_SH5:        return "IMAGE_FILE_MACHINE_SH5";
            case IMAGE_FILE_MACHINE_THUMB:      return "IMAGE_FILE_MACHINE_THUMB";
            case IMAGE_FILE_MACHINE_WCEMIPSV2:  return "IMAGE_FILE_MACHINE_WCEMIPSV2";
            default: return "unknown machine type";
        }
    }

    struct Header
    {
        uint16_t Machine;
        uint16_t NumberOfSections;
        uint32_t TimeDateStamp;
        uint32_t PointerToSymbolTable;
        uint32_t NumberOfSymbols;
        uint16_t SizeOfOptionalHeader;
        uint16_t Characteristics;
    };

    struct Context
    {
        Context() = default;
        Context(const Context&) = delete;

        struct
        {
            bool is_static = false;
            bool shared = false;
            int64_t thread_count = -1;
        } arg;

        std::vector<std::string_view> cmdline_args;
    };

    int do_main(int argc, char* argv[]);
}
