#include "platform.h"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <bcrypt.h>

#include <assert.h>

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

// Time

int64_t now_nsec() noexcept
{
    LARGE_INTEGER counter = { 0 }, frequency = { 0 };
    ::QueryPerformanceCounter(&counter);
    ::QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER now = { 0 };
    now.QuadPart = counter.QuadPart;
    now.QuadPart *= 1000LL * 1000LL * 1000LL;   // we need the time to be in nanoseconds (10^-9)
    now.QuadPart /= frequency.QuadPart;

    return now.QuadPart;
}

void get_process_times(int64_t& user_mode_nsec, int64_t& kernel_mode_nsec) noexcept
{
    const HANDLE handle = ::GetCurrentProcess();
    FILETIME creation_time, exit_time, kernel_time = { 0 }, user_time = { 0 };
    ::GetProcessTimes(handle, &creation_time, &exit_time, &kernel_time, &user_time);

    const ULARGE_INTEGER user{ user_time.dwLowDateTime, user_time.dwHighDateTime };
    user_mode_nsec = user.QuadPart * 100;   // user_time is the count of 100-nanosecond time units

    const ULARGE_INTEGER kernel{ kernel_time.dwLowDateTime, kernel_time.dwHighDateTime };
    kernel_mode_nsec = kernel.QuadPart * 100;   // kernel_time is the count of 100-nanosecond time units
}

// Cryptography

struct CryptographyContextData
{
    BCRYPT_ALG_HANDLE algorithm_handle = nullptr;
    BCRYPT_HASH_HANDLE hash_handle = nullptr;
    unsigned char* hash_object = nullptr;
    unsigned char* hash = nullptr;

    ~CryptographyContextData() noexcept
    {
        ::BCryptCloseAlgorithmProvider(algorithm_handle, 0);
        ::BCryptDestroyHash(hash_handle);
        ::free(hash_object);
        ::free(hash);
    }
};

CryptographyContext::~CryptographyContext() noexcept
{
    auto data = (CryptographyContextData*)this->data;
    if (data != nullptr)
        delete data;
    data = nullptr;
}

bool sha_256(const void* data, const size_t size, unsigned char* digest) noexcept
{
    CryptographyContext sha;

    return (begin_sha(sha) &&
            update_sha(sha, data, size) &&
            end_sha(sha, digest));
}

bool begin_sha(CryptographyContext& ctx) noexcept
{
    constexpr int SHA_DIGEST_SIZE = 32;

    ctx.data = new CryptographyContextData();
    auto ctx_data = (CryptographyContextData*)ctx.data;

    if (!BCRYPT_SUCCESS(::BCryptOpenAlgorithmProvider(&ctx_data->algorithm_handle, BCRYPT_SHA256_ALGORITHM, nullptr, 0)))
    {
        return false;
    }

    DWORD hash_object_size = 0;
    ULONG copied = 0;
    if (!BCRYPT_SUCCESS(::BCryptGetProperty(ctx_data->algorithm_handle, BCRYPT_OBJECT_LENGTH,
                                            (PUCHAR)&hash_object_size, sizeof(hash_object_size),
                                            &copied, 0)))
    {
        return false;
    }
    assert(copied != 0);
    ctx_data->hash_object = (unsigned char*)::malloc(hash_object_size);
    ctx_data->hash = (unsigned char*)::malloc(SHA_DIGEST_SIZE);

    if (!BCRYPT_SUCCESS(::BCryptCreateHash(ctx_data->algorithm_handle, &ctx_data->hash_handle,
                                           ctx_data->hash_object, hash_object_size,
                                           nullptr, 0, 0)))
    {
        return false;
    }

    return true;
}

bool update_sha(CryptographyContext& ctx, const void* data, const size_t size) noexcept
{
    auto ctx_data = (CryptographyContextData*)ctx.data;

    return BCRYPT_SUCCESS(::BCryptHashData(ctx_data->hash_handle, (PUCHAR)data, size, 0));
}

bool end_sha(CryptographyContext& ctx, unsigned char* digest) noexcept
{
    auto ctx_data = (CryptographyContextData*)ctx.data;

    return BCRYPT_SUCCESS(::BCryptFinishHash(ctx_data->hash_handle, digest, 32, 0));
}

bool generate_random_bytes(unsigned char* buffer, const size_t size) noexcept
{
    return BCRYPT_SUCCESS(::BCryptGenRandom(nullptr, buffer, size, BCRYPT_USE_SYSTEM_PREFERRED_RNG));
}
