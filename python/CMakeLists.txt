find_package(PkgConfig)
pkg_check_modules(PYTHON IMPORTED_TARGET python3-embed)
if (NOT PYTHON_FOUND)
    pkg_check_modules(PYTHON REQUIRED IMPORTED_TARGET python3)
endif (NOT PYTHON_FOUND)

find_program(PYTHON3_BIN "python3" REQUIRED)

add_library(
    py_yamal_obj
    STATIC
    "src/fmc++/python/config.cpp" # TODO: needed?
    "src/ytp.cpp"
)
target_link_libraries(
    py_yamal_obj
    PUBLIC
    fmc++
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
    #PY_EXTR_VER="${PROJECT_VERSION}"
    NO_DLL_DECORATOR
)

add_library(
    py_yamal_dep
    INTERFACE
)
target_link_libraries(
    py_yamal_dep
    INTERFACE
    fmc++
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
    fmc++
    ytp
    py_yamal
)
set_target_properties(
    yamal-python_module
    PROPERTIES
    OUTPUT_NAME "yamal"
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
else()
    set_property(TARGET yamal-python_module PROPERTY BUILD_RPATH_USE_ORIGIN TRUE)
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
    find_program(PYTHON3_BIN "python3" REQUIRED)
    # add_test(
    #     NAME extractor_py COMMAND

    #     "/bin/sh"
    #     "-c"
    #     "test-extractor-python"

    #     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python
    # )
    # set_tests_properties(
    #     extractor_py
    #     PROPERTIES
    #     ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python_dist/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${CMAKE_CURRENT_BINARY_DIR}/python_dist/build/${PYTHON3_SCRIPTS_DIR}:$ENV{PATH}"
    #     TIMEOUT 100
    # )
endif()
