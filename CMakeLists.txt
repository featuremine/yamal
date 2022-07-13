cmake_minimum_required(VERSION 3.15)

if (NOT DEFINED CMAKE_C_COMPILER)
    if (EXISTS "/opt/featuremine/tools/bin/x86_64-linux-musl-gcc")
        set(CMAKE_PREFIX_PATH "/opt/featuremine/tools")
        set(CMAKE_C_COMPILER "${CMAKE_PREFIX_PATH}/bin/x86_64-linux-musl-gcc")
        set(CMAKE_CXX_COMPILER "${CMAKE_PREFIX_PATH}/bin/x86_64-linux-musl-g++")
        set(CMAKE_FIND_ROOT_PATH "${CMAKE_PREFIX_PATH}")
        set(BUILD_SHARED_LIBS OFF)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
        message("using featuremine musl gcc compiler")
    else ()
        message("featuremine musl gcc not found")
    endif ()
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

file (STRINGS "meson.build" MESON_BUILD)
string(REGEX MATCH "project.*version:'([^']+)'" MESON_BUILD_VERSION "${MESON_BUILD}")

project(
    ytp
    VERSION "${CMAKE_MATCH_1}"
    DESCRIPTION "Featuremine YTP Library"
    HOMEPAGE_URL "https://www.featuremine.com"
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  string(APPEND CMAKE_CXX_FLAGS " -fconcepts -fpermissive")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -static-libgcc -static-libstdc++")
  string(APPEND CMAKE_SHARED_LINKER_FLAGS " -static-libgcc -static-libstdc++")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  string(APPEND CMAKE_C_FLAGS " -w -Qunused-arguments -Wpedantic")
  string(APPEND CMAKE_CXX_FLAGS " -Wpedantic -Wno-c++20-designator")
endif ()
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -Wl,--exclude-libs,ALL")
  string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--exclude-libs,ALL")
endif()

string(APPEND CMAKE_C_FLAGS " -pipe -fdiagnostics-color=always -Wduplicate-decl-specifier -Waddress -Warray-bounds -Wchar-subscripts -Winit-self -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wunused-function -Wunused-label -Wunused-variable -Winvalid-pch -Wall")
string(APPEND CMAKE_CXX_FLAGS " -pipe -fdiagnostics-color=always -Waddress -Warray-bounds -Wchar-subscripts -Winit-self -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wunused-function -Wunused-label -Wunused-variable -Winvalid-pch -Wall -Wnon-virtual-dtor")
add_definitions(-D_FILE_OFFSET_BITS=64)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

find_package(PkgConfig)
pkg_check_modules(GTEST REQUIRED IMPORTED_TARGET gtest)

add_library(
    fmc
    STATIC
    "${PROJECT_SOURCE_DIR}/src/fmc/cmp.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/error.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/extension.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/files.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/math.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/murmur3.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/process.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/signals.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/sockets.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/test.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/time.cpp"
)
target_include_directories(
    fmc
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
)
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(FMC_LIBS_PRIVATE "-lstdc++fs")
    set(FMCCPP_LIBS "-lstdc++fs")

    target_link_libraries(
        fmc
        PUBLIC
        stdc++fs
    )
endif()

add_library(
    fmc++
    STATIC
    "${PROJECT_SOURCE_DIR}/src/fmc++/config.cpp"
)
target_include_directories(
    fmc++
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
)

set(
    YTP_SRC
    "${PROJECT_SOURCE_DIR}/src/ytp/yamal.cpp"
    "${PROJECT_SOURCE_DIR}/src/ytp/peer.cpp"
    "${PROJECT_SOURCE_DIR}/src/ytp/channel.cpp"
    "${PROJECT_SOURCE_DIR}/src/ytp/time.cpp"
    "${PROJECT_SOURCE_DIR}/src/ytp/control.cpp"
    "${PROJECT_SOURCE_DIR}/src/ytp/sequence.cpp"
    "${PROJECT_SOURCE_DIR}/src/ytp/timeline.cpp"
)

add_library(
    ytp
    STATIC
    ${YTP_SRC}
)
target_include_directories(
    ytp
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
)
target_link_libraries(
    ytp
    PUBLIC
    fmc
    PRIVATE
    fmc++
)

add_library(
    ytp_shared
    SHARED
    ${YTP_SRC}
)
target_include_directories(
    ytp_shared
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
)
target_link_libraries(
    ytp_shared
    PUBLIC
    fmc
    PRIVATE
    fmc++
)
set_target_properties(
    ytp_shared
    PROPERTIES
    OUTPUT_NAME "ytp"
    LINKER_LANGUAGE "C"
)

enable_testing()

add_subdirectory(tests/fmc)
add_subdirectory(tests/fmc++)
add_subdirectory(tests/ytp)

set(YTP_VERSION "\"${PROJECT_VERSION}\"")
set(YTP_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(YTP_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(YTP_VERSION_PATCH ${PROJECT_VERSION_PATCH})

configure_file(
    "${PROJECT_SOURCE_DIR}/include/ytp/version.h.in"
    "${PROJECT_BINARY_DIR}/include/ytp/version.h"
    @ONLY
)

install(TARGETS ytp_shared ytp)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include"
    DESTINATION "."
    FILES_MATCHING
    PATTERN "*.h"
)
install(
    DIRECTORY "${PROJECT_BINARY_DIR}/include"
    DESTINATION "."
    FILES_MATCHING
    PATTERN "*.h"
)

configure_file(
    ${PROJECT_SOURCE_DIR}/lib/pkgconfig/ytp.pc.in
    ${PROJECT_BINARY_DIR}/ytp.pc
    @ONLY
)

install(
    FILES "${PROJECT_BINARY_DIR}/ytp.pc"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
)
