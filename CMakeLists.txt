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

option (BUILD_SHARED_LIBS "Request build of shared libraries." ON)
option (BUILD_TESTING "Enable build of the unit tests and their execution." ON)
option (BUILD_TOOLS "Enable build of command line tools." ON)
option (BUILD_WHEEL "Enable build of the python package." ON)
option (TEST_EXTENSIONS "Enable testing the extensions." ON)
option (BUILD_DOCUMENTATION "Build documentation." ON)
option (BUILD_DOCUMENTATION_FORCE "Force build documentation." OFF)

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

set(Subproject_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(BUILD_TESTING)
    find_package(Subproject REQUIRED)
    add_subproject(
        NAME libgtest
        GIT_REVISION "v1.14.0"
        GIT_URL "https://github.com/google/googletest.git"
        TARGETS gtest
    )
endif()

if (BUILD_TOOLS AND NOT TARGET tclap)
    find_package(Subproject REQUIRED)
    git_clone(
        GIT_REVISION "v1.2.5"
        GIT_URL "https://github.com/mirror/tclap.git"
        DIR "${CMAKE_BINARY_DIR}/dependencies/src/tclap"
    )
    add_library(tclap INTERFACE IMPORTED GLOBAL)
    target_include_directories(tclap INTERFACE "${CMAKE_BINARY_DIR}/dependencies/src/tclap/include")
endif()

set(YAMAL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(YAMAL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(YAMAL_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/bin")
set(YAMAL_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/${CMAKE_INSTALL_LIBDIR}")
set(package_FILES "")
set(package_DEP "")

set(FmConfig_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(FmConfig REQUIRED)
fm_config()

find_library(DL_LIB dl)
find_library(PTHREAD_LIB pthread)
include(GNUInstallDirs)

set(ReleaseTools_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(ReleaseTools REQUIRED)
get_python_platform(PYTHON_PLATFORM)

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
    "${PROJECT_BINARY_DIR}/package/include"
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
    "${PROJECT_BINARY_DIR}/package/include"
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
    "${PROJECT_BINARY_DIR}/package/include"
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
    ytp
    PROPERTIES
    OUTPUT_NAME "ytp"
    LIBRARY_OUTPUT_DIRECTORY "${YAMAL_LIB_DIR}"
)
set_target_properties(
    ytp-shared
    PROPERTIES
    OUTPUT_NAME "ytp"
    LIBRARY_OUTPUT_DIRECTORY "${YAMAL_LIB_DIR}"
)
if (BUILD_SHARED_LIBS)
    install(
        TARGETS
        ytp-shared
    )
    list(APPEND package_FILES "$<TARGET_FILE:ytp-shared>")
    list(APPEND package_DEP "ytp-shared")
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
    list(APPEND package_FILES "$<TARGET_FILE:ytp>")
    list(APPEND package_DEP "ytp")
endif ()

function(yamal_install_directory)
    cmake_parse_arguments(
        ARG
        ""
        "DIRECTORY;DESTINATION;PATTERN"
        ""
        ${ARGN}
    )
    install(${ARGN})
    file(
        GLOB_RECURSE FILES
        LIST_DIRECTORIES OFF
        RELATIVE "${ARG_DIRECTORY}/.."
        "${ARG_DIRECTORY}/${ARG_PATTERN}"
    )
    foreach (FILE ${FILES})
        set(FILE_DEST "${PROJECT_BINARY_DIR}/package/${ARG_DESTINATION}/${FILE}")
        cmake_path(NORMAL_PATH FILE_DEST)
        cmake_path(GET FILE_DEST PARENT_PATH FILE_DIR)
        add_custom_command(
            OUTPUT
            "${FILE_DEST}"

            COMMAND
            ${CMAKE_COMMAND} -E make_directory
            "${FILE_DIR}"

            COMMAND
            ${CMAKE_COMMAND} -E copy
            "${ARG_DIRECTORY}/../${FILE}"
            "${FILE_DEST}"

            DEPENDS
            "${ARG_DIRECTORY}/../${FILE}"
        )
        list(APPEND package_FILES "${FILE_DEST}")
        list(APPEND package_DEP "${FILE_DEST}")
        set(package_FILES ${package_FILES} PARENT_SCOPE)
        set(package_DEP ${package_DEP} PARENT_SCOPE)
    endforeach ()
endfunction()
yamal_install_directory(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/cmp"
    DESTINATION "./include"
    FILES_MATCHING PATTERN "*.h"
)
yamal_install_directory(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/fmc"
    DESTINATION "./include"
    FILES_MATCHING PATTERN "*.h"
)
yamal_install_directory(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/fmc++"
    DESTINATION "./include"
    FILES_MATCHING PATTERN "*.hpp"
)
yamal_install_directory(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/ytp"
    DESTINATION "./include"
    FILES_MATCHING PATTERN "*.h"
)
yamal_install_directory(
    DIRECTORY "${PROJECT_SOURCE_DIR}/src/fmc++"
    DESTINATION "./src"
    FILES_MATCHING PATTERN "*.cpp"
)

set(YTP_VERSION "\"${PROJECT_VERSION}\"")
set(YTP_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(YTP_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(YTP_VERSION_PATCH ${PROJECT_VERSION_PATCH})
configure_file(
    "${PROJECT_SOURCE_DIR}/include/ytp/version.h.in"
    "${PROJECT_BINARY_DIR}/package/include/ytp/version.h"
    @ONLY
)
install(
    FILES "${PROJECT_BINARY_DIR}/package/include/ytp/version.h"
    DESTINATION "./include/ytp"
)
list(APPEND package_FILES "${PROJECT_BINARY_DIR}/package/include/ytp/version.h")
list(APPEND package_DEP "${PROJECT_BINARY_DIR}/package/include/ytp/version.h")

configure_file(
    "${PROJECT_SOURCE_DIR}/lib/pkgconfig/ytp.pc.in"
    "${YAMAL_LIB_DIR}/pkgconfig/ytp.pc"
    @ONLY
)
install(
    FILES "${YAMAL_LIB_DIR}/pkgconfig/ytp.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
list(APPEND package_FILES "${YAMAL_LIB_DIR}/pkgconfig/ytp.pc")
list(APPEND package_DEP "${YAMAL_LIB_DIR}/pkgconfig/ytp.pc")

if(BUILD_TOOLS)
    function(add_tool)
        cmake_parse_arguments(
            ARG
            ""
            "NAME"
            "LINK_LIBRARIES"
            ${ARGN}
        )
        add_executable(${ARG_NAME} "src/tools/${ARG_NAME}.cpp")
        target_link_libraries(${ARG_NAME} ${ARG_LINK_LIBRARIES})
        set_target_properties(
            ${ARG_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${YAMAL_BIN_DIR}"
        )
        install(TARGETS ${ARG_NAME})
        list(APPEND package_FILES "$<TARGET_FILE:${ARG_NAME}>")
        list(APPEND package_DEP "${ARG_NAME}")
        set(package_FILES ${package_FILES} PARENT_SCOPE)
        set(package_DEP ${package_DEP} PARENT_SCOPE)
    endfunction()
    add_tool(
        NAME yamal-run
        LINK_LIBRARIES PRIVATE ytp fmc++ tclap
    )
    add_tool(
        NAME yamal-latency
        LINK_LIBRARIES PRIVATE ytp fmc++ tclap
    )
    add_tool(
        NAME yamal-replay
        LINK_LIBRARIES PRIVATE ytp fmc++ tclap
    )
    add_tool(
        NAME yamal-cp
        LINK_LIBRARIES PRIVATE ytp fmc++ tclap
    )
    add_tool(
        NAME yamal-daemon
        LINK_LIBRARIES PRIVATE ytp fmc++ tclap
     )
    add_tool(
        NAME yamal-tail
        LINK_LIBRARIES PRIVATE ytp fmc++ tclap
     )
    add_tool(
        NAME yamal-local-perf
        LINK_LIBRARIES PRIVATE ytp fmc++ tclap
     )
    add_tool(
        NAME yamal-perf
        LINK_LIBRARIES PRIVATE ytp fmc++ tclap
     )
    add_tool(
        NAME yamal-stats
        LINK_LIBRARIES PRIVATE ytp fmc++ tclap
     )
endif()

add_subdirectory(python)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/output")
add_custom_command(
    OUTPUT
    "${CMAKE_BINARY_DIR}/output/yamal-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}.tar.gz"

    COMMAND
    ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_BINARY_DIR}/package"
    ${CMAKE_COMMAND} -E tar "czvf" "${CMAKE_BINARY_DIR}/output/yamal-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}.tar.gz"
    ${package_FILES}

    DEPENDS
    ${package_DEP}
)

add_custom_target(
    yamal-package ALL
    DEPENDS
    "${CMAKE_BINARY_DIR}/output/yamal-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}.tar.gz"
)

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
