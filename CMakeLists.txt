cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

file (STRINGS "VERSION" BUILD_NUMBER)
project(
    ytp
    VERSION "${BUILD_NUMBER}"
    DESCRIPTION "Featuremine Yamal Library"
    HOMEPAGE_URL "https://www.featuremine.com"
)

option (BUILD_SHARED_LIBS "Request build of shared libraries." ON)
option (BUILD_TESTING "Enable build of the unit tests and their execution." ON)

# Add the cmake folder so the FindSphinx module is found
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Add subdir "docs" for Doxygen + Sphinx to use.
add_subdirectory ("docs")


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

string(APPEND CMAKE_C_FLAGS " -fPIC -pipe -fdiagnostics-color=always -Wduplicate-decl-specifier -Waddress -Warray-bounds -Wchar-subscripts -Winit-self -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wunused-function -Wunused-label -Wunused-variable -Winvalid-pch -Wall")
string(APPEND CMAKE_CXX_FLAGS " -fPIC -pipe -fdiagnostics-color=always -Waddress -Warray-bounds -Wchar-subscripts -Winit-self -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wunused-function -Wunused-label -Wunused-variable -Winvalid-pch -Wall -Wnon-virtual-dtor")
add_definitions(-D_FILE_OFFSET_BITS=64)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

find_package(Threads REQUIRED)

find_package(PkgConfig)
pkg_check_modules(GTEST REQUIRED IMPORTED_TARGET gtest)
pkg_check_modules(TCLAP REQUIRED IMPORTED_TARGET tclap)

include(ExternalProject)
###############################################################################
# Apache Portable Runtime (APR)
###############################################################################
set(APR_TARGET APR)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  ExternalProject_Add(
    ${APR_TARGET}
    PREFIX ${APR_TARGET}

    GIT_REPOSITORY "https://github.com/apache/apr.git"
    GIT_TAG "1.7.0"
    UPDATE_COMMAND ""

    TEST_BEFORE_INSTALL true
    TEST_COMMAND        make -j4 test
  )
else ()
  ExternalProject_Add(
    ${APR_TARGET}
    PREFIX ${APR_TARGET}

    GIT_REPOSITORY "https://github.com/apache/apr.git"
    GIT_TAG "1.7.0"
    UPDATE_COMMAND ""

    PATCH_COMMAND     ./buildconf
    CONFIGURE_COMMAND CFLAGS=-fPIC <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --enable-static --disable-shared
    TEST_COMMAND        ""
  )
endif()

ExternalProject_Get_Property(${APR_TARGET} INSTALL_DIR)
set(APR_INSTALL_DIR ${INSTALL_DIR})
set(APR_INCLUDE_DIR ${APR_INSTALL_DIR}/include/apr-1)
set(APR_LIB_DIR ${APR_INSTALL_DIR}/lib)
add_library (libapr STATIC IMPORTED)
set_target_properties (libapr PROPERTIES IMPORTED_LOCATION ${APR_LIB_DIR}/libapr-1.a)
###############################################################################

set(
    YTP_SRC
    "${PROJECT_SOURCE_DIR}/src/errno.cpp"
    "${PROJECT_SOURCE_DIR}/src/yamal.cpp"
    "${PROJECT_SOURCE_DIR}/src/peer.cpp"
    "${PROJECT_SOURCE_DIR}/src/channel.cpp"
    "${PROJECT_SOURCE_DIR}/src/time.cpp"
    "${PROJECT_SOURCE_DIR}/src/control.cpp"
    "${PROJECT_SOURCE_DIR}/src/sequence.cpp"
    "${PROJECT_SOURCE_DIR}/src/timeline.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils.cpp"
)

add_library(
    ytp_obj
    OBJECT
    ${YTP_SRC}
)
target_include_directories(
    ytp_obj
    PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
    "${APR_INCLUDE_DIR}"
)
target_link_libraries(
    ytp_obj
    PUBLIC
    libapr
    Threads::Threads
)
add_dependencies(ytp_obj ${APR_TARGET})

add_library(
    ytp
    STATIC
    $<TARGET_OBJECTS:ytp_obj>
)
target_include_directories(
    ytp
    PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
    "${APR_INCLUDE_DIR}"
)
target_link_libraries(
    ytp
    PUBLIC
    libapr
    Threads::Threads
)

add_library(
    ytp_shared
    SHARED
    $<TARGET_OBJECTS:ytp_obj>
)
if(BUILD_SHARED_LIBS)
    target_include_directories(
        ytp_shared
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_BINARY_DIR}/include"
        "${APR_INCLUDE_DIR}"
    )
    target_link_libraries(
        ytp_shared
        PUBLIC
        libapr
        Threads::Threads
    )
    set_target_properties(
        ytp_shared
        PROPERTIES
        OUTPUT_NAME "ytp"
        LINKER_LANGUAGE "C"
    )
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
add_subdirectory(bin)

set(YTP_VERSION "\"${PROJECT_VERSION}\"")
set(YTP_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(YTP_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(YTP_VERSION_PATCH ${PROJECT_VERSION_PATCH})

configure_file(
    "${PROJECT_SOURCE_DIR}/include/ytp/version.h.in"
    "${PROJECT_BINARY_DIR}/include/ytp/version.h"
    @ONLY
)

if(BUILD_SHARED_LIBS)
    install(
        TARGETS
        ytp_shared
    )
endif()

install(
    TARGETS
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
