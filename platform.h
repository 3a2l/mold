#pragma once

#include <string>
#include <string_view>

// Filesystem

std::string get_self_path() noexcept;

std::string get_current_dir() noexcept;

bool set_current_dir(const std::string_view directory) noexcept;

std::string create_temporary_file(const std::string_view directory) noexcept;

// Memory

enum MapMode
{
    MAP_MODE_READ = 1 << 0,
    MAP_MODE_WRITE = 1 << 1,
    MAP_MODE_READWRITE = MAP_MODE_READ | MAP_MODE_WRITE
};

void* map_memory(const size_t size) noexcept;

void* map_memory(const std::string_view filepath, const MapMode mode, const size_t size) noexcept;

void unmap_memory(void* address, size_t length) noexcept;

// Time

int64_t now_nsec() noexcept;

void get_process_times(int64_t& user_mode_nsec, int64_t& kernel_mode_nsec) noexcept;

// Cryptography

struct CryptographyContext
{
    void* data = nullptr;
    ~CryptographyContext() noexcept;
};

bool sha_256(const void* data, const size_t size, unsigned char* digest) noexcept;

bool begin_sha(CryptographyContext& ctx) noexcept;

bool update_sha(CryptographyContext& ctx, const void* data, const size_t size) noexcept;

bool end_sha(CryptographyContext& ctx, unsigned char* digest) noexcept;

bool generate_random_bytes(unsigned char* buffer, const size_t size) noexcept;
