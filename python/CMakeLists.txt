find_package(PkgConfig)
pkg_check_modules(PYTHON IMPORTED_TARGET python3-embed)
if (NOT PYTHON_FOUND)
    pkg_check_modules(PYTHON REQUIRED IMPORTED_TARGET python3)
endif (NOT PYTHON_FOUND)

find_program(PYTHON3_BIN "python3" REQUIRED)

add_library(
    py_yamal_obj
    STATIC
    "src/ytp.cpp"
)
target_link_libraries(
    py_yamal_obj
    PUBLIC
    ytp
)
target_include_directories(
    py_yamal_obj
    PUBLIC
    "${YAMAL_INC_DIR}"
    "${YAMAL_PYTHON_INC_DIR}"
    "${PYTHON_INCLUDE_DIRS}"
)
target_compile_definitions(
    py_yamal_obj
    PRIVATE
    YTP_VERSION="${PROJECT_VERSION}"
    NO_DLL_DECORATOR
)

add_library(
    py_yamal_dep
    INTERFACE
)
target_link_libraries(
    py_yamal_dep
    INTERFACE
    ytp
)
target_include_directories(
    py_yamal_dep
    INTERFACE
    "${YAMAL_INC_DIR}"
    "${YAMAL_PYTHON_INC_DIR}"
    "${PYTHON_INCLUDE_DIRS}"
)

add_library(
    py_yamal
    STATIC
    $<TARGET_OBJECTS:py_yamal_obj>
)
target_link_libraries(
    py_yamal
    PRIVATE
    py_yamal_dep
)
set_target_properties(
    py_yamal
    PROPERTIES
    OUTPUT_NAME "py_yamal"
    LIBRARY_OUTPUT_DIRECTORY "${YAMAL_LIB_DIR}"
)
install(
    TARGETS
    py_yamal
)

add_library(
    yamal-python_module
    SHARED
    "src/main.c"
)
target_link_libraries(
    yamal-python_module
    PRIVATE
    ytp
    py_yamal
)
set_target_properties(
    yamal-python_module
    PROPERTIES
    OUTPUT_NAME "ytp"
    PREFIX ""
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python_dist/yamal"
)
target_include_directories(
    yamal-python_module
    PRIVATE
    "${YAMAL_INC_DIR}"
    "${YAMAL_PYTHON_INC_DIR}"
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
)
set_target_properties(
    yamal-sys_base_module
    PROPERTIES
    OUTPUT_NAME "sys_base"
    PREFIX ""
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python_dist/yamal"
)
target_include_directories(
    yamal-sys_base_module
    PRIVATE
    "${YAMAL_INC_DIR}"
    "${YAMAL_PYTHON_INC_DIR}"
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
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python_dist/yamal"
)
target_include_directories(
    yamal-python_testmodule
    PRIVATE
    "${YAMAL_INC_DIR}"
)

file(
    COPY ${PROJECT_SOURCE_DIR}/python/include
    DESTINATION "${CMAKE_BINARY_DIR}/python_dist/yamal"
    FILES_MATCHING
        PATTERN .h
        PATTERN .hpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_options(yamal-python_module PRIVATE -Wl,-rpath,@loader_path)
    target_link_options(yamal-python_module PRIVATE -undefined dynamic_lookup)
    target_link_options(yamal-sys_base_module PRIVATE -Wl,-rpath,@loader_path)
    target_link_options(yamal-sys_base_module PRIVATE -undefined dynamic_lookup)
else()
    set_property(TARGET yamal-python_module PROPERTY BUILD_RPATH_USE_ORIGIN TRUE)
    set_property(TARGET yamal-sys_base_module PROPERTY BUILD_RPATH_USE_ORIGIN TRUE)
endif()

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
)

file(
    GLOB PYI_FILES
    "${PROJECT_SOURCE_DIR}/python/src/*.pyi"
    "${PROJECT_SOURCE_DIR}/python/src/*.py"
)
file(
    COPY ${PYI_FILES}
    DESTINATION "${CMAKE_BINARY_DIR}/python_dist/yamal"
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
    "--build-base=${CMAKE_BINARY_DIR}/python_dist/build"
    ${DEBUG_FLAG}

    "egg_info"
    "--egg-base" "${CMAKE_BINARY_DIR}/python_dist"

    "bdist_wheel"
    "--bdist-dir=${CMAKE_BINARY_DIR}/python_dist/bdist"
    "--dist-dir=${CMAKE_BINARY_DIR}/output"

    "--plat-name=${PYTHON_PLATFORM}"

    COMMAND
    "${PYTHON3_BIN}" "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"

    "build"
    "--build-base=${CMAKE_BINARY_DIR}/python_dist/build"
    "--build-lib=${CMAKE_BINARY_DIR}/python_dist/build/lib"
    ${DEBUG_FLAG}

    "build_scripts"
    "--executable=${PYTHON3_BIN}"
    "-f"

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/python_dist

    DEPENDS yamal-python_module yamal-sys_base_module yamal-python_testmodule
)

if(BUILD_TESTING)
    add_subdirectory(tests)
    find_program(PYTHON3_BIN "python3" REQUIRED)
    add_test(
        NAME yamal_py COMMAND

        "/bin/sh"
        "-c"
        "test-yamal-python"

        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(
        yamal_py
        PROPERTIES
        ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/python_dist:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${CMAKE_BINARY_DIR}/python_dist/build/${PYTHON3_SCRIPTS_DIR}:$ENV{PATH}"
        TIMEOUT 100
    )
endif()
