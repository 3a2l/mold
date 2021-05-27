cmake_minimum_required(VERSION 3.0)

project(mold
        VERSION 0.1.1)

###############################################################################
# Project-wide options
###############################################################################
set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

###############################################################################
# Third-party dependencies
###############################################################################
# pthreads
if (UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
endif()

# zlib
add_subdirectory(thirdparty/zlib)

# LibreSSL
set(LIBRESSL_APPS  OFF CACHE BOOL "Build LibreSSL apps")
set(LIBRESSL_TESTS OFF CACHE BOOL "Build LibreSSL tests")
add_subdirectory(thirdparty/libressl-3.3.3)

# oneTBB
# set(TBB_TEST OFF CACHE BOOL "Enable oneTBB testing")
include(thirdparty/oneTBB/cmake/TBBBuild.cmake)
tbb_build(TBB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/oneTBB/
          CONFIG_DIR ${CMAKE_CURRENT_BINARY_DIR})
find_package(TBB REQUIRED
             CONFIG
             PATHS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/oneTBB/cmake)

# xxHash
add_subdirectory(thirdparty/xxHash/cmake_unofficial)

# mimalloc
set(MI_BUILD_TESTS  OFF CACHE BOOL "Build mimalloc test executables.")
set(MI_BUILD_SHARED OFF CACHE BOOL "Build shared library")
set(MI_BUILD_OBJECT OFF CACHE BOOL "Build object library")
add_subdirectory(thirdparty/mimalloc)

###############################################################################
# Project configuration
###############################################################################

find_package(Git REQUIRED)

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

###############################################################################
# Targets
###############################################################################

add_executable(mold)

target_sources(mold
    PRIVATE
        arch_i386.cc
        arch_x86_64.cc
        archive_file.cc
        cmdline.cc
        compress.cc
        filepath.cc
        gc_sections.cc
        glob.cc
        icf.cc
        input_sections.cc
        linker_script.cc
        main.cc
        mapfile.cc
        object_file.cc
        output_chunks.cc
        output_file.cc
        passes.cc
        perf.cc
        subprocess.cc
        symbols.cc
        tar.cc)

target_compile_definitions(mold
    PRIVATE
        "GIT_HASH=\"${GIT_HASH}\""
        "MOLD_VERSION=\"${PROJECT_VERSION}\"")

target_link_libraries(mold
    PRIVATE
        crypto
        TBB::tbb
        zlib
        xxHash::xxhash
        mimalloc-static
        $<$<PLATFORM_ID:Linux>:dl>
        $<$<PLATFORM_ID:Linux>:Threads::Threads>)

add_library(mold-wrapper SHARED)

target_sources(mold-wrapper
    PRIVATE
        mold-wrapper.c)