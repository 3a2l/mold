#include "platform.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

static
uint32_t get_umask()
{
    u32 orig_umask = umask(0);
    umask(orig_umask);
    return orig_umask;
}

std::string create_temporary_file(const std::string_view directory)
{
    std::string filepath;
    return filepath;
}

void* map_memory(const std::string_view filepath, const MapMode mode, const size_t size) noexcept
{

}

void unmap_memory(void* address, size_t length) noexcept
{
    ::munmap(address, length);
}
