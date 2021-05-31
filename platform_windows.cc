#include "platform.h"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>

#include <string>
#include <assert.h>
#include <string_view>

constexpr DWORD MAX_PATH_BUFFER_SIZE = MAX_PATH + 1;

// Strings

static inline
std::wstring string_utf8_to_utf16(const std::string_view input) noexcept
{
    const int length = ::MultiByteToWideChar(CP_UTF8, 0, input.data(), (int)input.size(), nullptr, 0);
    std::wstring result(length, 0);
    ::MultiByteToWideChar(CP_UTF8, 0, input.data(), (int)input.size(), result.data(), length);
    return result;
}

static inline
std::string string_utf16_to_utf8(const std::wstring_view input) noexcept
{
    const int length = ::WideCharToMultiByte(CP_UTF8, 0, input.data(), (int)input.size(), nullptr, 0, nullptr, nullptr);
    std::string result(length, 0);
    ::WideCharToMultiByte(CP_UTF8, 0, input.data(), (int)input.size(), result.data(), length, nullptr, nullptr);
    return result;
}

// Filesystem

std::string get_self_path() noexcept
{
    wchar_t buffer[MAX_PATH_BUFFER_SIZE] = { 0 };
    ::GetModuleFileName(nullptr, buffer, MAX_PATH_BUFFER_SIZE);

    return string_utf16_to_utf8(buffer);
}

std::string get_current_dir() noexcept
{
    wchar_t buffer[MAX_PATH_BUFFER_SIZE] = { 0 };
    ::GetCurrentDirectory(MAX_PATH_BUFFER_SIZE, buffer);

    return string_utf16_to_utf8(buffer);
}

bool set_current_dir(const std::string_view directory) noexcept
{
    const std::wstring directory_utf16 = string_utf8_to_utf16(directory);
    return ::SetCurrentDirectory(directory_utf16.c_str()) != 0;
}

std::string create_temporary_file(const std::string_view directory) noexcept
{
    const std::wstring directory_utf16 = string_utf8_to_utf16(directory);

    wchar_t temp_file[MAX_PATH_BUFFER_SIZE] = { 0 };
    ::GetTempFileName(directory_utf16.c_str(), L"mld", 0, temp_file);

    return string_utf16_to_utf8(temp_file);
}

// Memory

void* map_memory(const size_t size) noexcept
{
    return ::VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void* map_memory(const std::string_view filepath, const MapMode mode, const size_t size) noexcept
{
    // Convert mode to windows flags
    DWORD file_access = 0, map_page_protection = 0, map_access = 0;
    switch (mode)
    {
    case MAP_MODE_READ:
        file_access = GENERIC_READ;
        map_page_protection = PAGE_READONLY;
        map_access = FILE_MAP_READ;
        break;

    default:
        file_access = GENERIC_ALL;
        map_page_protection = PAGE_READWRITE;
        map_access = FILE_MAP_ALL_ACCESS;
    }

    // Open the file
    const std::wstring filepath_utf16 = string_utf8_to_utf16(filepath);
    const HANDLE file_handle = ::CreateFile(filepath_utf16.c_str(), file_access,
                                            0, nullptr,
                                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    assert(file_handle != INVALID_HANDLE_VALUE);

    // If we are using the file to write, resize to fit before mapping
    if (mode != MAP_MODE_READ)
    {
        // TODO: Handle `size` greater than 32 bits
        ::SetFilePointer(file_handle, size, 0, FILE_BEGIN);
        ::SetEndOfFile(file_handle);
    }

    // Create a file mapping
    const HANDLE map_handle = ::CreateFileMapping(file_handle, nullptr, map_page_protection, 0, 0, nullptr);
    assert(map_handle != nullptr);
    ::CloseHandle(file_handle);

    // Map file to memory
    void* view = ::MapViewOfFile(map_handle, map_access, 0, 0, size);
    // TODO: is it safe to close the map handle now, or should we carry this around?
    ::CloseHandle(map_handle);

    return view;
}

void unmap_memory(void* address, size_t length) noexcept
{
    ::UnmapViewOfFile(address);
}
