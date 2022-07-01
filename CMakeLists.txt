cmake_minimum_required(VERSION 3.15)

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
  string(APPEND CMAKE_CXX_FLAGS " -fconcepts -fpermissive -fno-use-cxa-atexit")
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

if (NOT TARGET fmc)
    pkg_check_modules(FMC REQUIRED IMPORTED_TARGET fmc)
    pkg_check_modules(FMCPP REQUIRED IMPORTED_TARGET fmc++)
    add_library(
        fmc
        INTERFACE IMPORTED
    )
    target_link_libraries(
        fmc
        INTERFACE
        PkgConfig::FMC
    )
    add_library(
        fmc++
        INTERFACE IMPORTED
    )
    target_link_libraries(
        fmc++
        INTERFACE
        PkgConfig::FMCPP
    )
endif ()

set(
    YTP_SRC
    "${PROJECT_SOURCE_DIR}/src/yamal.cpp"
    "${PROJECT_SOURCE_DIR}/src/peer.cpp"
    "${PROJECT_SOURCE_DIR}/src/channel.cpp"
    "${PROJECT_SOURCE_DIR}/src/time.cpp"
    "${PROJECT_SOURCE_DIR}/src/control.cpp"
    "${PROJECT_SOURCE_DIR}/src/sequence.cpp"
    "${PROJECT_SOURCE_DIR}/src/timeline.cpp"
)

add_library(
    ytp_obj
    OBJECT
    ${YTP_SRC}
)
target_include_directories(
    ytp_obj
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
)
target_link_libraries(
    ytp_obj
    PUBLIC
    fmc
    PRIVATE
    fmc++
)

add_library(
    ytp
    STATIC
    $<TARGET_OBJECTS:ytp_obj>
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
)

add_library(
    ytp_shared
    SHARED
    $<TARGET_OBJECTS:ytp_obj>
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
)
set_target_properties(
    ytp_shared
    PROPERTIES
    OUTPUT_NAME "ytp"
    LINKER_LANGUAGE "C"
)

enable_testing()

add_subdirectory(tests)
add_subdirectory(python)

set(YTP_VERSION "\"${PROJECT_VERSION}\"")
set(YTP_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(YTP_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(YTP_VERSION_PATCH ${PROJECT_VERSION_PATCH})

configure_file(
    "${PROJECT_SOURCE_DIR}/include/ytp/version.h.in"
    "${PROJECT_BINARY_DIR}/include/ytp/version.h"
    @ONLY
)

install(
    TARGETS
    ytp_shared
    ytp
)

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
    ${PROJECT_SOURCE_DIR}/share/pkgconfig/ytp.pc.in
    ${PROJECT_BINARY_DIR}/ytp.pc
    @ONLY
)

install(
    FILES "${PROJECT_BINARY_DIR}/ytp.pc"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
)
