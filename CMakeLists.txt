#[===[
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
]===]

cmake_minimum_required(VERSION 3.15)

file (STRINGS "VERSION" YAMAL_VERSION)

project(
    yamal
    VERSION "${YAMAL_VERSION}"
    DESCRIPTION "Featuremine YTP Library"
    HOMEPAGE_URL "https://www.featuremine.com"
)

set(Subproject_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Subproject REQUIRED)

option (BUILD_SHARED_LIBS "Request build of shared libraries." ON)
option (BUILD_TESTING "Enable build of the unit tests and their execution." ON)
option (BUILD_TOOLS "Enable build of command line tools." ON)
option (BUILD_WHEEL "Enable build of the python package." ON)
option (TEST_EXTENSIONS "Enable testing the extensions." ON)
option (BUILD_DOCUMENTATION "Build documentation." ON)

if (BUILD_DOCUMENTATION)
    set(FmDocumentation_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    find_package(FmDocumentation REQUIRED)

    add_documentation(
        NAME yamal-docs
        BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs"
        SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs"
        TITLE "Yamal Documentation"
        VERSION "${PROJECT_VERSION}"
    )
endif()

if(BUILD_TESTING)
    add_subproject(
        NAME libgtest
        GIT_REVISION "v1.14.0"
        GIT_URL "https://github.com/google/googletest.git"
        TARGETS gtest
    )
endif()

set(YAMAL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(YAMAL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(YAMAL_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/bin")
set(YAMAL_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/lib")

set(FmConfig_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(FmConfig REQUIRED)
fm_config()

find_library(DL_LIB dl)
find_library(PTHREAD_LIB pthread)
include(GNUInstallDirs)

add_library(
    fmc++
    STATIC
    "${PROJECT_SOURCE_DIR}/src/fmc++/config.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc++/websocket.cpp"
)
target_include_directories(
    fmc++
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
)
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(
        fmc++
        PUBLIC
        stdc++fs
    )
endif()

set(
    FMC_SRC
    "${PROJECT_SOURCE_DIR}/src/cmp/cmp.c"
    "${PROJECT_SOURCE_DIR}/src/murmur3/murmur3.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/component.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/cmdline.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/config.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/config.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/decimal128.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/error.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/extension.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/files.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/math.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/memory.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/prio_queue.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/process.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/rational64.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/reactor.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/rprice.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/signals.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/sockets.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/string.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/test.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/time.cpp"
    "${PROJECT_SOURCE_DIR}/src/libdecnumber/decQuad.c"
    "${PROJECT_SOURCE_DIR}/src/libdecnumber/decContext.c"
)

set(
    YTP_SRC
    "${PROJECT_SOURCE_DIR}/src/ytp/api.cpp"
    "${PROJECT_SOURCE_DIR}/src/ytp/yamal.c"
    "${PROJECT_SOURCE_DIR}/src/ytp/time.cpp"
    "${PROJECT_SOURCE_DIR}/src/ytp/control.cpp"
    "${PROJECT_SOURCE_DIR}/src/ytp/sequence.cpp"
    "${PROJECT_SOURCE_DIR}/src/ytp/timeline.cpp"
    "${PROJECT_SOURCE_DIR}/src/ytp/announcement.c"
    "${PROJECT_SOURCE_DIR}/src/ytp/subscription.c"
    "${PROJECT_SOURCE_DIR}/src/ytp/index.c"
    "${PROJECT_SOURCE_DIR}/src/ytp/stream.c"
    "${PROJECT_SOURCE_DIR}/src/ytp/streams.c"
    "${PROJECT_SOURCE_DIR}/src/ytp/data.c"
    "${PROJECT_SOURCE_DIR}/src/ytp/cursor.c"
    "${PROJECT_SOURCE_DIR}/src/ytp/glob.cpp"
)

add_library(
    fmc_obj
    OBJECT
    ${FMC_SRC}
)
target_include_directories(
    fmc_obj
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    PRIVATE
    "${PROJECT_SOURCE_DIR}/include/libdecnumber"
)
target_link_libraries(
    fmc_obj
    PRIVATE
    fmc++
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
    PRIVATE
    fmc++
)

add_library(
    ytp_dep
    INTERFACE
)
target_include_directories(
    ytp_dep
    INTERFACE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
)

add_library(
    ytp
    STATIC
    $<TARGET_OBJECTS:fmc_obj>
    $<TARGET_OBJECTS:ytp_obj>
)
target_link_libraries(
    ytp

    PRIVATE
    fmc++

    PUBLIC
    ytp_dep
)

add_library(
    ytp-shared
    SHARED
    $<TARGET_OBJECTS:fmc_obj>
    $<TARGET_OBJECTS:ytp_obj>
)
target_link_libraries(
    ytp-shared
    PUBLIC
    ytp_dep
)
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(
        ytp
        PRIVATE
        ${DL_LIB}
        ${PTHREAD_LIB}
    )
    target_link_libraries(
        ytp-shared
        PRIVATE
        ${DL_LIB}
        ${PTHREAD_LIB}
    )
endif()
set_target_properties(
    ytp-shared
    PROPERTIES
    OUTPUT_NAME "ytp"
)
if (BUILD_SHARED_LIBS)
    install(
        TARGETS
        ytp-shared
    )
else ()
    set_target_properties(
        ytp-shared
        PROPERTIES
        EXCLUDE_FROM_ALL ON
    )
    if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        string(APPEND YTP_PRIVATE_LIBS " -lstdc++fs -ldl -lpthread")
    endif()
    install(
        TARGETS
        ytp
    )
endif ()

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/cmp"
    DESTINATION "./include"
    FILES_MATCHING
    PATTERN "*.h"
)
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/fmc"
    DESTINATION "./include"
    FILES_MATCHING
    PATTERN "*.h"
)
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/fmc++"
    DESTINATION "./include"
    FILES_MATCHING
    PATTERN "*.hpp"
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
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/src/fmc++"
    DESTINATION "./src"
    FILES_MATCHING
    PATTERN "*.cpp"
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
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

if(BUILD_TOOLS)
    add_executable(
        yamal-run
        "src/tools/yamal-run.cpp"
    )
    target_link_libraries(
        yamal-run
        PRIVATE
        ytp
        fmc++
    )
    install(
        TARGETS
        yamal-run
    )

    add_executable(
       yamal-latency
       "src/tools/yamal-latency.cpp"
    )
    target_link_libraries(
       yamal-latency
       PRIVATE
       ytp
       fmc++
    )
    install(
       TARGETS
       yamal-latency
    )

    add_executable(
       yamal-replay
       "src/tools/yamal-replay.cpp"
    )
    target_link_libraries(
       yamal-replay
       PRIVATE
       ytp
       fmc++
    )
    install(
       TARGETS
       yamal-replay
    )

    add_executable(
       yamal-cp
       "src/tools/yamal-cp.cpp"
    )
    target_link_libraries(
       yamal-cp
       PRIVATE
       ytp
       fmc++
    )
    install(
       TARGETS
       yamal-cp
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
     )
    install(
        TARGETS
        yamal-tail
    )


    add_executable(
        yamal-local-perf
        "src/tools/yamal-local-perf.cpp"
    )
    target_link_libraries(
        yamal-local-perf
        PRIVATE
        ytp
        fmc++
     )
    install(
        TARGETS
        yamal-local-perf
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
     )
    install(
        TARGETS
        yamal-stats
    )
endif()

add_subdirectory(python)

if(BUILD_TESTING)
    enable_testing()
    find_program( MEMORYCHECK_COMMAND valgrind )
    set(MEMORYCHECK_COMMAND_OPTIONS "--log-fd=2 --verbose --trace-children=yes --leak-check=full --show-leak-kinds=all --track-origins=yes")
    set(VALGRIND_COMMAND_OPTIONS "--log-fd=2 --verbose --trace-children=yes --leak-check=full --show-leak-kinds=all --track-origins=yes")
    include(CTest)
    add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_VENDOR "Featuremine")
set(CPACK_PACKAGE_CONTACT "support@featuremine.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)
