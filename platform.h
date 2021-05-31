#pragma once

#include <string>
#include <string_view>

enum MapMode
{
    MAP_MODE_READ   = 1 << 0,
    MAP_MODE_WRITE  = 1 << 1,
    MAP_MODE_READWRITE = MAP_MODE_READ | MAP_MODE_WRITE
};

// Filesystem

std::string get_self_path() noexcept;

std::string get_current_dir() noexcept;

bool set_current_dir(const std::string_view directory) noexcept;

std::string create_temporary_file(const std::string_view directory) noexcept;

// Memory

void* map_memory(const size_t size) noexcept;

void* map_memory(const std::string_view filepath, const MapMode mode, const size_t size) noexcept;

void unmap_memory(void* address, size_t length) noexcept;
