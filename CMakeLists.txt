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

set(Subproject_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option (BUILD_SHARED_LIBS "Request build of shared libraries." ON)
option (BUILD_TESTING "Enable build of the unit tests and their execution." ON)
option (BUILD_TOOLS "Enable build of command line tools." ON)
option (BUILD_API_DOCS "Enable build of the Doxygen API documentation." ON)
option (BUILD_API_DOCS_FORCE "Enable force build of the Doxygen API documentation." OFF)
option (BUILD_PACKAGE "Enable build of the self-extracting package." ON)
option (BUILD_WHEEL "Enable build of the python package." ON)
option (TEST_EXTENSIONS "Enable testing the extensions." ON)

find_program(PYTHON3_BIN "python3" REQUIRED)

set(YAMAL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(YAMAL_PYTHON_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/python/include")
set(YAMAL_PYTHON_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/python/src")
set(YAMAL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(YAMAL_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/bin")
set(YAMAL_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/lib")

set(FmConfig_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(FmConfig REQUIRED)
fm_config()
execute_process(COMMAND ${PYTHON3_BIN} -c "import sys;sys.stdout.write('scripts-%d.%d' % sys.version_info[:2])" OUTPUT_VARIABLE PYTHON3_SCRIPTS_DIR)
include(GNUInstallDirs)

find_package(Doxygen)
if (BUILD_API_DOCS_FORCE OR (DOXYGEN_FOUND AND BUILD_API_DOCS))
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

find_library(DL_LIB dl)
find_library(PTHREAD_LIB pthread)
include(GNUInstallDirs)

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
    FMC_SRC
    "${PROJECT_SOURCE_DIR}/src/cmp/cmp.c"
    "${PROJECT_SOURCE_DIR}/src/murmur3/murmur3.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/component.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/config.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/decimal128.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/error.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/extension.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/files.cpp"
    "${PROJECT_SOURCE_DIR}/src/fmc/math.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/memory.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/prio_queue.c"
    "${PROJECT_SOURCE_DIR}/src/fmc/process.cpp"
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
    ${FMC_SRC}
    ${YTP_SRC}
)
target_include_directories(
    ytp_obj
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
    PRIVATE
    "${PROJECT_SOURCE_DIR}/include/libdecnumber"
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
    $<TARGET_OBJECTS:ytp_obj>
)
target_link_libraries(
    ytp

    PRIVATE
    fmc++

    PUBLIC
    ytp_dep
)
if (BUILD_SHARED_LIBS)
    add_library(
        ytp-shared
        SHARED
        $<TARGET_OBJECTS:ytp_obj>
    )
    target_link_libraries(
        ytp-shared
        PUBLIC
        ytp_dep
    )
    if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        target_link_libraries(
            ytp-shared
            PRIVATE
            stdc++fs
            ${DL_LIB}
            ${PTHREAD_LIB}
        )
    endif()
    set_target_properties(
        ytp-shared
        PROPERTIES
        OUTPUT_NAME "ytp"
    )
    install(
        TARGETS
        ytp-shared
    )
else()
    if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        string(APPEND YTP_PRIVATE_LIBS " -lstdc++fs -ldl -lpthread")
    endif()
    install(
        TARGETS
        ytp
    )
endif ()
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(
        ytp
        PRIVATE
        stdc++fs
        ${DL_LIB}
        ${PTHREAD_LIB}
    )
endif()

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/fmc"
    DESTINATION "./include"
    FILES_MATCHING
    PATTERN "*.h"
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

include(CPack)

add_subdirectory(python)

file(
    COPY ${PROJECT_SOURCE_DIR}/python/include
    DESTINATION "${CMAKE_BINARY_DIR}/python_dist/yamal"
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

file(
    COPY ${PROJECT_SOURCE_DIR}/python/tests
    DESTINATION "${CMAKE_BINARY_DIR}/python_dist/yamal"
    FILES_MATCHING
        PATTERN "*.py"
        PATTERN "fmc++" EXCLUDE
        PATTERN "yamal" EXCLUDE
)

file(
    COPY ${PROJECT_SOURCE_DIR}/scripts
    DESTINATION "${CMAKE_BINARY_DIR}/python_dist"
)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(DEBUG_FLAG "--debug")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PYTHON_PLATFORM "manylinux_2_17_${CMAKE_SYSTEM_PROCESSOR}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    execute_process(COMMAND sw_vers -productVersion
                    OUTPUT_VARIABLE MACOS_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    STRING(REGEX REPLACE "^([0-9]+)\\.?([0-9]+)\\.?([0-9]+)?$" "\\1" MACOS_VERSION_MAJOR "${MACOS_VERSION}")
    STRING(REGEX REPLACE "^([0-9]+)\\.?([0-9]+)\\.?([0-9]+)?$" "\\2" MACOS_VERSION_MINOR "${MACOS_VERSION}")
    set(PYTHON_PLATFORM "macosx_${MACOS_VERSION_MAJOR}_0_${CMAKE_SYSTEM_PROCESSOR}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PYTHON_PLATFORM "win_${CMAKE_SYSTEM_PROCESSOR}")
else()
    message(FATAL_ERROR "Platform not supported")
endif()

add_custom_target(
    yamal-whl ALL

    COMMAND
    "${PYTHON3_BIN}" "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"

    "build"
    "--build-base=${CMAKE_CURRENT_BINARY_DIR}/python_dist/build"
    ${DEBUG_FLAG}

    "egg_info"
    "--egg-base" "${CMAKE_CURRENT_BINARY_DIR}/python_dist"

    "bdist_wheel"
    "--bdist-dir=${CMAKE_CURRENT_BINARY_DIR}/python_dist/bdist"
    "--dist-dir=${CMAKE_BINARY_DIR}/output"

    "--plat-name=${PYTHON_PLATFORM}"

    COMMAND
    "${PYTHON3_BIN}" "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"

    "build"
    "--build-base=${CMAKE_CURRENT_BINARY_DIR}/python_dist/build"
    "--build-lib=${CMAKE_CURRENT_BINARY_DIR}/python_dist/build/lib"
    ${DEBUG_FLAG}

    "build_scripts"
    "--executable=${PYTHON3_BIN}"
    "-f"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python_dist

    DEPENDS yamal-python_module
)
