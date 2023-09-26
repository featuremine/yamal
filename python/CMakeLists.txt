#[===[
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
]===]

find_package(PkgConfig)
pkg_search_module(PYTHON IMPORTED_TARGET python3-embed python3)

set(YAMAL_PYTHON_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(
    py_yamal_obj
    OBJECT
    "src/fmc++/python/config.cpp"
)
target_include_directories(
    py_yamal_obj
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(
    py_yamal_obj
    PRIVATE
    fmc++
    PkgConfig::PYTHON
)

add_library(
    py_yamal_dep
    INTERFACE
)
target_include_directories(
    py_yamal_dep
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
add_library(
    py_yamal
    STATIC
    $<TARGET_OBJECTS:py_yamal_obj>
)
target_link_libraries(
    py_yamal

    PUBLIC
    py_yamal_dep
)

add_library(
    yamal-python_module
    SHARED
    "src/main.c"
    "src/ytp.cpp"
)
target_link_libraries(
    yamal-python_module
    PRIVATE
    ytp
    py_yamal_dep
)
target_compile_definitions(
    yamal-python_module
    PRIVATE
    NO_DLL_DECORATOR
)
set_target_properties(
    yamal-python_module
    PROPERTIES
    OUTPUT_NAME "ytp"
    PREFIX ""
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/dist/yamal"
)
target_include_directories(
    yamal-python_module
    PRIVATE
    "${YAMAL_INC_DIR}"
    "${PYTHON_INCLUDE_DIRS}"
)

add_library(
    yamal-sys_base_module
    SHARED
    "src/sys_base.c"
)
target_link_libraries(
    yamal-sys_base_module
    PRIVATE
    ytp
    py_yamal_dep
)
target_compile_definitions(
    yamal-sys_base_module
    PRIVATE
    NO_DLL_DECORATOR
)
set_target_properties(
    yamal-sys_base_module
    PROPERTIES
    OUTPUT_NAME "sys_base"
    PREFIX ""
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/dist/yamal"
)
target_include_directories(
    yamal-sys_base_module
    PRIVATE
    "${YAMAL_INC_DIR}"
    "${PYTHON_INCLUDE_DIRS}"
)

add_library(
    yamal8_module
    SHARED
    "src/yamal8.cpp"
)
target_link_libraries(
    yamal8_module
    PRIVATE
    ytp
    py_yamal_dep
)
target_compile_definitions(
    yamal8_module
    PRIVATE
    NO_DLL_DECORATOR
)
set_target_properties(
    yamal8_module
    PROPERTIES
    OUTPUT_NAME "yamal8"
    PREFIX ""
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/dist/yamal"
)
target_include_directories(
    yamal8_module
    PRIVATE
    "${YAMAL_INC_DIR}"
    "${PYTHON_INCLUDE_DIRS}"
)

add_library(
    yamal-python_testmodule
    SHARED
    "src/testmodule.c"
)
target_link_libraries(
    yamal-python_testmodule
    PRIVATE
    ytp
)
set_target_properties(
    yamal-python_testmodule
    PROPERTIES
    OUTPUT_NAME "testmodule"
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/dist/yamal"
)
target_include_directories(
    yamal-python_testmodule
    PRIVATE
    "${YAMAL_INC_DIR}"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_options(yamal-python_module PRIVATE -Wl,-rpath,@loader_path)
    target_link_options(yamal-python_module PRIVATE -undefined dynamic_lookup)
    target_link_options(yamal-sys_base_module PRIVATE -Wl,-rpath,@loader_path)
    target_link_options(yamal-sys_base_module PRIVATE -undefined dynamic_lookup)
    target_link_options(yamal8_module PRIVATE -Wl,-rpath,@loader_path)
    target_link_options(yamal8_module PRIVATE -undefined dynamic_lookup)
else()
    set_property(TARGET yamal-python_module PROPERTY BUILD_RPATH_USE_ORIGIN TRUE)
    set_property(TARGET yamal-sys_base_module PROPERTY BUILD_RPATH_USE_ORIGIN TRUE)
    set_property(TARGET yamal8_module PROPERTY BUILD_RPATH_USE_ORIGIN TRUE)
endif()

file(
    COPY ${PROJECT_SOURCE_DIR}/python/include
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist/yamal"
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

file(
    COPY ${PROJECT_SOURCE_DIR}/python/tests
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist/yamal"
    FILES_MATCHING
        PATTERN "*.py"
        PATTERN "fmc++" EXCLUDE
)

file(
    GLOB PYI_FILES
    "${PROJECT_SOURCE_DIR}/python/src/*.pyi"
    "${PROJECT_SOURCE_DIR}/python/src/*.py"
)
file(
    COPY ${PYI_FILES}
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist/yamal"
)

file(
    COPY ${PROJECT_SOURCE_DIR}/python/scripts
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist"
)

file(
    COPY "${PROJECT_SOURCE_DIR}/LICENSE"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist"
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

if(BUILD_WHEEL)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}-py")
    find_program(PYTHON3_BIN "python3")
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/output/yamal-${PROJECT_VERSION}-py3-none-manylinux_2_17_x86_64.whl"

        COMMAND
        "${PYTHON3_BIN}" "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"

        "build"
        "--build-base=${CMAKE_CURRENT_BINARY_DIR}/buildwheel"
        "--build-lib=${CMAKE_CURRENT_BINARY_DIR}/buildwheel/lib"
        ${DEBUG_FLAG}

        "egg_info"
        "--egg-base" "${CMAKE_CURRENT_BINARY_DIR}"

        "bdist_wheel"
        "--bdist-dir=${CMAKE_CURRENT_BINARY_DIR}/bdist"
        "--dist-dir=${CMAKE_BINARY_DIR}/output"

        "--plat-name=${PYTHON_PLATFORM}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist

        DEPENDS yamal-python_module yamal-sys_base_module yamal-python_testmodule yamal8_module
    )

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}-py/yamal.egg-info/SOURCES.txt"
    
        COMMAND
        "${PYTHON3_BIN}" "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"

        "build_scripts"
        "--executable=${PYTHON3_BIN}"

        "egg_info"
        "--egg-base" "${CMAKE_CURRENT_BINARY_DIR}-py"

        "build"
        "--build-base=${CMAKE_CURRENT_BINARY_DIR}/build"
        "--build-lib=${CMAKE_CURRENT_BINARY_DIR}/build/lib"
        "--build-scripts=${CMAKE_CURRENT_BINARY_DIR}/build/scripts"
        ${DEBUG_FLAG}

        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist

        DEPENDS yamal-python_module yamal-sys_base_module yamal-python_testmodule yamal8_module
    )
    add_custom_target(
        yamal-whl ALL
        DEPENDS "${CMAKE_BINARY_DIR}/output/yamal-${PROJECT_VERSION}-py3-none-manylinux_2_17_x86_64.whl"
    )
    add_custom_target(
        yamal-py ALL
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}-py/yamal.egg-info/SOURCES.txt"
    )
    set(WHEEL_yamal_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "Wheel path for yamal" FORCE)
    set(WHEEL_yamal_SCRIPTS "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "Wheel script path for yamal" FORCE)
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
    if(TEST_EXTENSIONS)
        if(BUILD_WHEEL)
            set(WheelPackage_DIR "${PROJECT_SOURCE_DIR}/cmake")
            find_package(WheelPackage REQUIRED)
            test_python_package(
                NAME yamal_py
                PACKAGE yamal
                TIMEOUT 100
                TEST "test-yamal-python"
            )
        endif()
    endif()
endif()
