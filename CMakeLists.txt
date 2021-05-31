cmake_minimum_required(VERSION 3.0)

project(mold
        LANGUAGES C CXX
        VERSION 0.1.1)

###############################################################################
# Project-wide options
###############################################################################
set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

###############################################################################
# Third-party dependencies
###############################################################################
# pthreads
if (UNIX AND NOT APPLE)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# zlib
add_subdirectory(thirdparty/zlib)

# LibreSSL
set(LIBRESSL_APPS  OFF CACHE BOOL "Build LibreSSL apps")
set(LIBRESSL_TESTS OFF CACHE BOOL "Build LibreSSL tests")
add_subdirectory(thirdparty/libressl-3.3.3)

# oneTBB
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
list(APPEND mi_cflags "-DMI_USE_ENVIRON=0")
add_subdirectory(thirdparty/mimalloc)

###############################################################################
# Project configuration
###############################################################################

option(MOLD_TESTS "Enable Mold tests." ON)

find_package(Git REQUIRED)

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

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
        elf.h
        filepath.cc
        gc_sections.cc
        glob.cc
        icf.cc
        input_sections.cc
        linker_script.cc
        main.cc
        mapfile.cc
        mold.h
        object_file.cc
        output_chunks.cc
        output_file.cc
        passes.cc
        perf.cc
        platform.h
        $<$<PLATFORM_ID:Windows>:platform_windows.cc>
        $<$<PLATFORM_ID:Linux>:platform_linux.cc>
        subprocess.cc
        symbols.cc
        tar.cc)

target_compile_definitions(mold
    PRIVATE
        "GIT_HASH=\"${GIT_HASH}\""
        "MOLD_VERSION=\"${PROJECT_VERSION}\""
        $<$<PLATFORM_ID:Windows>:UNICODE;_UNICODE;NOMINMAX>)

target_compile_options(mold
    PRIVATE
        $<$<PLATFORM_ID:Linux>:-Wno-deprecated-volatile>
        $<$<CXX_COMPILER_ID:MSVC>:/MP>)

target_link_options(mold
    PRIVATE
        $<$<PLATFORM_ID:Linux>:-Wl,-whole-archive $<TARGET_FILE:mimalloc-static> -Wl,-no-whole-archive;-Wl,-as-needed>)

target_link_libraries(mold
    PRIVATE
        crypto
        TBB::tbb
        zlib
        xxHash::xxhash
        $<$<PLATFORM_ID:Linux>:dl>
        $<$<PLATFORM_ID:Linux>:Threads::Threads>
        $<$<CXX_COMPILER_ID:MSVC>:Dbghelp.lib>)

add_dependencies(mold mimalloc-static)

if (NOT WIN32)
    add_library(mold-wrapper SHARED)

    target_sources(mold-wrapper
        PRIVATE
            mold-wrapper.c)

    target_link_libraries(mold-wrapper
        PRIVATE
            $<$<PLATFORM_ID:Linux>:dl>)
endif()

###############################################################################
# Tests
###############################################################################
if (MOLD_TESTS)
    enable_testing()

    if (UNIX)
        set(MOLD_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test/linux)

        add_test(NAME allow-multiple-definition
                 COMMAND ${MOLD_TEST_DIR}/allow-multiple-definition.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME as-needed
                 COMMAND ${MOLD_TEST_DIR}/as-needed.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME auxiliary
                 COMMAND ${MOLD_TEST_DIR}/auxiliary.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME basic
                 COMMAND ${MOLD_TEST_DIR}/basic.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME bsymbolic-functions
                 COMMAND ${MOLD_TEST_DIR}/bsymbolic-functions.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME bsymbolic
                 COMMAND ${MOLD_TEST_DIR}/bsymbolic.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME build-id
                 COMMAND ${MOLD_TEST_DIR}/build-id.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME canonical-plt
                 COMMAND ${MOLD_TEST_DIR}/canonical-plt.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME comment
                 COMMAND ${MOLD_TEST_DIR}/comment.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME common
                 COMMAND ${MOLD_TEST_DIR}/common.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME compress-debug-sections
                 COMMAND ${MOLD_TEST_DIR}/compress-debug-sections.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME compressed-debug-info
                 COMMAND ${MOLD_TEST_DIR}/compressed-debug-info.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME copyrel-protected
                 COMMAND ${MOLD_TEST_DIR}/copyrel-protected.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME copyrel-relro
                 COMMAND ${MOLD_TEST_DIR}/copyrel-relro.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME copyrel
                 COMMAND ${MOLD_TEST_DIR}/copyrel.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME defs
                 COMMAND ${MOLD_TEST_DIR}/defs.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME demangle
                 COMMAND ${MOLD_TEST_DIR}/demangle.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME discard
                 COMMAND ${MOLD_TEST_DIR}/discard.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME dt_init
                 COMMAND ${MOLD_TEST_DIR}/dt_init.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME duplicate-error
                 COMMAND ${MOLD_TEST_DIR}/duplicate-error.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME dynamic-linker
                 COMMAND ${MOLD_TEST_DIR}/dynamic-linker.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME dynamic
                 COMMAND ${MOLD_TEST_DIR}/dynamic.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME entry
                 COMMAND ${MOLD_TEST_DIR}/entry.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME exception
                 COMMAND ${MOLD_TEST_DIR}/exception.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME exclude-libs2
                 COMMAND ${MOLD_TEST_DIR}/exclude-libs2.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME exclude-libs
                 COMMAND ${MOLD_TEST_DIR}/exclude-libs.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME execstack
                 COMMAND ${MOLD_TEST_DIR}/execstack.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME export-dynamic
                 COMMAND ${MOLD_TEST_DIR}/export-dynamic.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME export-from-exe
                 COMMAND ${MOLD_TEST_DIR}/export-from-exe.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME fatal-warnings
                 COMMAND ${MOLD_TEST_DIR}/fatal-warnings.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME filler
                 COMMAND ${MOLD_TEST_DIR}/filler.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME filter
                 COMMAND ${MOLD_TEST_DIR}/filter.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME gc-sections
                 COMMAND ${MOLD_TEST_DIR}/gc-sections.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME gnu-hash
                 COMMAND ${MOLD_TEST_DIR}/gnu-hash.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME hello-dynamic
                 COMMAND ${MOLD_TEST_DIR}/hello-dynamic.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME hello-static
                 COMMAND ${MOLD_TEST_DIR}/hello-static.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME help
                 COMMAND ${MOLD_TEST_DIR}/help.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME i386-hello-dynamic
                 COMMAND ${MOLD_TEST_DIR}/i386-hello-dynamic.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME i386-hello-static
                 COMMAND ${MOLD_TEST_DIR}/i386-hello-static.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME i386-tlsdesc
                 COMMAND ${MOLD_TEST_DIR}/i386-tlsdesc.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME i386-tls-gd
                 COMMAND ${MOLD_TEST_DIR}/i386-tls-gd.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME i386-tls-ld
                 COMMAND ${MOLD_TEST_DIR}/i386-tls-ld.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME icf
                 COMMAND ${MOLD_TEST_DIR}/icf.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME ifunc-dso
                 COMMAND ${MOLD_TEST_DIR}/ifunc-dso.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME ifunc-dynamic
                 COMMAND ${MOLD_TEST_DIR}/ifunc-dynamic.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME ifunc-static
                 COMMAND ${MOLD_TEST_DIR}/ifunc-static.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME image-base
                 COMMAND ${MOLD_TEST_DIR}/image-base.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME init_array
                 COMMAND ${MOLD_TEST_DIR}/init_array.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME initfirst
                 COMMAND ${MOLD_TEST_DIR}/initfirst.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME interpose
                 COMMAND ${MOLD_TEST_DIR}/interpose.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME library-path-dynamic
                 COMMAND ${MOLD_TEST_DIR}/library-path-dynamic.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME library-path-static
                 COMMAND ${MOLD_TEST_DIR}/library-path-static.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME linker-script
                 COMMAND ${MOLD_TEST_DIR}/linker-script.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME many-sections
                 COMMAND ${MOLD_TEST_DIR}/many-sections.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME mergeable-strings
                 COMMAND ${MOLD_TEST_DIR}/mergeable-strings.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME missing-but-ok
                 COMMAND ${MOLD_TEST_DIR}/missing-but-ok.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME missing-error
                 COMMAND ${MOLD_TEST_DIR}/missing-error.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME mold-wrapper
                 COMMAND ${MOLD_TEST_DIR}/mold-wrapper.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold-wrapper>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME nocopyreloc
                 COMMAND ${MOLD_TEST_DIR}/nocopyreloc.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME no-quick-exit
                 COMMAND ${MOLD_TEST_DIR}/no-quick-exit.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME note-property
                 COMMAND ${MOLD_TEST_DIR}/note-property.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME note
                 COMMAND ${MOLD_TEST_DIR}/note.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME now
                 COMMAND ${MOLD_TEST_DIR}/now.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME omagic
                 COMMAND ${MOLD_TEST_DIR}/omagic.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME pie
                 COMMAND ${MOLD_TEST_DIR}/pie.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME pltgot
                 COMMAND ${MOLD_TEST_DIR}/pltgot.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME plt
                 COMMAND ${MOLD_TEST_DIR}/plt.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME preload
                 COMMAND ${MOLD_TEST_DIR}/preload.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME protected
                 COMMAND ${MOLD_TEST_DIR}/protected.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME push-pop-state
                 COMMAND ${MOLD_TEST_DIR}/push-pop-state.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME relax
                 COMMAND ${MOLD_TEST_DIR}/relax.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME reloc-overflow
                 COMMAND ${MOLD_TEST_DIR}/reloc-overflow.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME reloc
                 COMMAND ${MOLD_TEST_DIR}/reloc.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME relro
                 COMMAND ${MOLD_TEST_DIR}/relro.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME repro
                 COMMAND ${MOLD_TEST_DIR}/repro.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME response-file
                 COMMAND ${MOLD_TEST_DIR}/response-file.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME retain-symbols-file
                 COMMAND ${MOLD_TEST_DIR}/retain-symbols-file.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME rpath
                 COMMAND ${MOLD_TEST_DIR}/rpath.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME run
                 COMMAND ${MOLD_TEST_DIR}/run.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold> $<TARGET_FILE:mold-wrapper>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME shared
                 COMMAND ${MOLD_TEST_DIR}/shared.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME spare-dynamic-tags
                 COMMAND ${MOLD_TEST_DIR}/spare-dynamic-tags.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME static-archive
                 COMMAND ${MOLD_TEST_DIR}/static-archive.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME stdout
                 COMMAND ${MOLD_TEST_DIR}/stdout.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME strip
                 COMMAND ${MOLD_TEST_DIR}/strip.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME symtab
                 COMMAND ${MOLD_TEST_DIR}/symtab.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME thin-archive
                 COMMAND ${MOLD_TEST_DIR}/thin-archive.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME tlsdesc
                 COMMAND ${MOLD_TEST_DIR}/tlsdesc.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME tls-dso
                 COMMAND ${MOLD_TEST_DIR}/tls-dso.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME tls-gd
                 COMMAND ${MOLD_TEST_DIR}/tls-gd.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME tls-ld
                 COMMAND ${MOLD_TEST_DIR}/tls-ld.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME tls-nopic
                 COMMAND ${MOLD_TEST_DIR}/tls-nopic.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME tls-pic
                 COMMAND ${MOLD_TEST_DIR}/tls-pic.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME trace
                 COMMAND ${MOLD_TEST_DIR}/trace.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME trace-symbol
                 COMMAND ${MOLD_TEST_DIR}/trace-symbol.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME undefined
                 COMMAND ${MOLD_TEST_DIR}/undefined.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME version-script2
                 COMMAND ${MOLD_TEST_DIR}/version-script2.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME version-script3
                 COMMAND ${MOLD_TEST_DIR}/version-script3.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME version-script4
                 COMMAND ${MOLD_TEST_DIR}/version-script4.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME version-script
                 COMMAND ${MOLD_TEST_DIR}/version-script.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME version
                 COMMAND ${MOLD_TEST_DIR}/version.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME warn-common
                 COMMAND ${MOLD_TEST_DIR}/warn-common.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME weak-export
                 COMMAND ${MOLD_TEST_DIR}/weak-export.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME whole-archive
                 COMMAND ${MOLD_TEST_DIR}/whole-archive.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})

        add_test(NAME wrap
                 COMMAND ${MOLD_TEST_DIR}/wrap.sh ${CMAKE_CURRENT_BINARY_DIR}/Testing $<TARGET_FILE:mold>
                 WORKING_DIRECTORY ${MOLD_TEST_DIR})
    endif()
endif()
