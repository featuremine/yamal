cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

file (STRINGS "VERSION" YAMAL_VERSION)

project(
    yamal
    VERSION "${YAMAL_VERSION}"
    DESCRIPTION "Featuremine YTP Library"
    HOMEPAGE_URL "https://www.featuremine.com"
)

option (BUILD_SHARED_LIBS "Request build of shared libraries." ON)
option (BUILD_TESTING "Enable build of the unit tests and their execution." ON)
option (BUILD_TOOLS "Enable build of command line tools." ON)
option (BUILD_API_DOCS "Enable build of the Doxygen API documentation." ON)
option (BUILD_API_DOCS_FORCE "Enable force build of the Doxygen API documentation." OFF)

# Add the cmake folder so the FindSphinx module is found
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(Doxygen)
if (BUILD_API_DOCS_FORCE OR (DOXYGEN_FOUND AND BUILD_API_DOCS))
    find_package(Doxygen)
    # Add subdir "docs" for Doxygen + Sphinx to use.
    add_subdirectory ("docs")
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    string(APPEND CMAKE_CXX_FLAGS " -fconcepts -fpermissive")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -static-libgcc -static-libstdc++")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -static-libgcc -static-libstdc++")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    string(APPEND CMAKE_C_FLAGS " -w -Qunused-arguments -Wpedantic")
    string(APPEND CMAKE_CXX_FLAGS " -Wpedantic -Wno-c++20-designator")
endif ()
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--exclude-libs,ALL")
endif()

string(APPEND CMAKE_C_FLAGS " -Wduplicate-decl-specifier -Waddress -Warray-bounds -Wchar-subscripts -Winit-self -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wunused-function -Wunused-label -Wunused-variable -Winvalid-pch -Wall")
string(APPEND CMAKE_CXX_FLAGS " -Waddress -Warray-bounds -Wchar-subscripts -Winit-self -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wunused-function -Wunused-label -Wunused-variable -Winvalid-pch -Wall -Wnon-virtual-dtor")
add_definitions(-D_FILE_OFFSET_BITS=64)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_library(
    fmc
    STATIC
    "${PROJECT_SOURCE_DIR}/src/cmp/cmp.c"
    "${PROJECT_SOURCE_DIR}/src/murmur3/murmur3.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/component.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/config.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/error.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/extension.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/files.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/math.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/process.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/reactor.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/signals.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/sockets.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/string.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/test.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/time.cpp"
)
target_include_directories(
    fmc
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
)
find_library(DL_LIB dl)
find_library(PTHREAD_LIB pthread)
target_link_libraries(
    fmc
    PRIVATE
    ${DL_LIB}
    ${PTHREAD_LIB}
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
install(
    TARGETS
    fmc
)
configure_file(
    "${PROJECT_SOURCE_DIR}/lib/pkgconfig/fmc.pc.in"
    "${PROJECT_BINARY_DIR}/fmc.pc"
    @ONLY
)
install(
    FILES "${PROJECT_BINARY_DIR}/fmc.pc"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
)
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/fmc"
    DESTINATION "./include"
    FILES_MATCHING
    PATTERN "*.h"
)

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
configure_file(
    "${PROJECT_SOURCE_DIR}/lib/pkgconfig/fmc++.pc.in"
    "${PROJECT_BINARY_DIR}/fmc++.pc"
    @ONLY
)
install(
    FILES "${PROJECT_BINARY_DIR}/fmc++.pc"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
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
set_target_properties(
    ytp
    PROPERTIES
    LINKER_LANGUAGE "C"
)
install(
    TARGETS
    ytp
)
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/ytp"
    DESTINATION "./include"
    FILES_MATCHING
    PATTERN "*.h"
)
install(
    DIRECTORY "${PROJECT_BINARY_DIR}/include/ytp"
    DESTINATION "./include"
    FILES_MATCHING
    PATTERN "*.h"
)
set(YTP_VERSION "\"${PROJECT_VERSION}\"")
set(YTP_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(YTP_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(YTP_VERSION_PATCH ${PROJECT_VERSION_PATCH})
configure_file(
    "${PROJECT_SOURCE_DIR}/include/ytp/version.h.in"
    "${PROJECT_BINARY_DIR}/include/ytp/version.h"
    @ONLY
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

if(BUILD_TOOLS)
    find_package(PkgConfig)
    pkg_check_modules(TCLAP REQUIRED IMPORTED_TARGET tclap)

    add_executable(
        yamal-run
        "src/tools/yamal-run.cpp"
    )
    target_link_libraries(
        yamal-run
        PRIVATE
        ytp
        fmc++
        fmc
        PkgConfig::TCLAP
        pthread
    )
    install(
        TARGETS
        yamal-run
    )

    add_executable(
        yamal-daemon
        "src/tools/yamal-daemon.cpp"
    )
    target_link_libraries(
        yamal-daemon
        PRIVATE
        ytp
        fmc++
        fmc
        pthread
    )
    install(
        TARGETS
        yamal-daemon
    )

    add_executable(
        yamal-tail
        "src/tools/yamal-tail.cpp"
    )
    target_link_libraries(
        yamal-tail
        PRIVATE
        ytp
        fmc++
        fmc
        pthread
    )
    install(
        TARGETS
        yamal-tail
    )

    add_executable(
        yamal-perf
        "src/tools/yamal-perf.cpp"
    )
    target_link_libraries(
        yamal-perf
        PRIVATE
        ytp
        fmc++
        fmc
        pthread
    )
    install(
        TARGETS
        yamal-perf
    )

    add_executable(
        yamal-stats
        "src/tools/yamal-stats.cpp"
    )
    target_link_libraries(
        yamal-stats
        PRIVATE
        ytp
        fmc++
        fmc
        pthread
    )
    install(
        TARGETS
        yamal-stats
    )
endif()
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_VENDOR "Featuremine")
set(CPACK_PACKAGE_CONTACT "support@featuremine.com")
set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

include(CPack)
