#include "platform.h"

#include <time.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#include <linux/limits.h>

#include <openssl/sha.h>
#include <openssl/rand.h>

constexpr int MAX_PATH_BUFFER_SIZE = PATH_MAX + 1;

static
uint32_t get_umask()
{
    uint32_t orig_umask = ::umask(0);
    ::umask(orig_umask);
    return orig_umask;
}

// Filesystem

std::string get_self_path() noexcept
{
    char buffer[MAX_PATH_BUFFER_SIZE] = { 0 };
    size_t n = ::readlink("/proc/self/exe", buffer, sizeof(buffer));
    if (n == -1)
        return "";
    return { buffer, n };
}

std::string get_current_dir() noexcept
{
    char buffer[MAX_PATH_BUFFER_SIZE] = { 0 };
    ::getcwd(buffer, sizeof(buffer));
    return buffer;
}

bool set_current_dir(const std::string_view directory) noexcept
{
    return ::chdir(directory.data()) == 0;
}

std::string create_temporary_file(const std::string_view directory) noexcept
{
    std::string temp_file(directory);
    temp_file += "/.mold-XXXXXX";

    ::mkstemp(temp_file.data());
    return temp_file;
}

// Memory

void* map_memory(const size_t size) noexcept
{
    auto address = ::mmap(nullptr, size,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS,
                          -1, 0);
    if (address == MAP_FAILED)
    {
        return nullptr;
    }

    return address;
}

void* map_memory(const std::string_view filepath, const MapMode mode, const size_t size) noexcept
{
    int file_access = 0, map_page_protection = 0, map_access = 0;

    switch (mode)
    {
    case MapMode::MAP_MODE_READ:
        file_access = O_RDONLY;
        map_page_protection = PROT_READ;
        map_access = MAP_PRIVATE;
        break;

    default:
        file_access = O_RDWR | O_CREAT;
        map_page_protection = PROT_READ | PROT_WRITE;
        map_access = MAP_SHARED;
    }

    int fd = ::open(filepath.data(), file_access);
    if (fd == -1)
    {
        return nullptr;
    }

    // If we are using the file to write, resize to fit before mapping
    if (mode != MAP_MODE_READ)
    {
        ::ftruncate(fd, size);
        ::fchmod(fd, (0777 & ~get_umask()));
    }

    auto address = ::mmap(nullptr, size,
                          map_page_protection,
                          map_access, fd, 0);
    if (address == MAP_FAILED)
    {
        ::close(fd);
        return nullptr;
    }

    ::close(fd);
    return address;
}

void unmap_memory(void* address, size_t length) noexcept
{
    ::munmap(address, length);
}

// Time

int64_t now_nsec() noexcept
{
    struct timespec t;
    ::clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)t.tv_sec * 1000000000 + t.tv_nsec;
}

void get_process_times(int64_t& user_mode_nsec, int64_t& kernel_mode_nsec) noexcept
{
    struct rusage usage;
    ::getrusage(RUSAGE_SELF, &usage);

    // ru_utime is expressed in a timeval structure (seconds plus microseconds)
    user_mode_nsec = (int64_t) usage.ru_utime.tv_sec * 1000000000 + usage.ru_utime.tv_usec * 1000;

    // ru_stime is expressed in a timeval structure (seconds plus microseconds)
    kernel_mode_nsec = (int64_t) usage.ru_stime.tv_sec * 1000000000 + usage.ru_stime.tv_usec * 1000;
}

// Cryptography

struct CryptographyContextData
{
    SHA256_CTX ctx;
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
    CryptographyContext ctx;

    return (begin_sha(ctx) &&
            update_sha(ctx, data, size) &&
            end_sha(ctx, digest));
}

bool begin_sha(CryptographyContext& ctx) noexcept
{
    ctx.data = new CryptographyContextData();
    auto ctx_data = (CryptographyContextData*)ctx.data;

    return SHA256_Init(&ctx_data->ctx) == 1;
}

bool update_sha(CryptographyContext& ctx, const void* data, const size_t size) noexcept
{
    auto ctx_data = (CryptographyContextData*)ctx.data;

    return SHA256_Update(&ctx_data->ctx, data, size) == 1;
}

bool end_sha(CryptographyContext& ctx, unsigned char* digest) noexcept
{
    auto ctx_data = (CryptographyContextData*)ctx.data;

    return SHA256_Final(digest, &ctx_data->ctx) == 1;
}

bool generate_random_bytes(unsigned char* buffer, const size_t size) noexcept
{
    return RAND_bytes(buffer, size) == 1;
}