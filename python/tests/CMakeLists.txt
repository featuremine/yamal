find_package(PkgConfig)
pkg_check_modules(GTEST REQUIRED IMPORTED_TARGET gtest)
pkg_check_modules(PYTHON IMPORTED_TARGET python3-embed)
if (NOT PYTHON_FOUND)
    pkg_check_modules(PYTHON REQUIRED IMPORTED_TARGET python3)
endif (NOT PYTHON_FOUND)

macro(yamal_unit_test)
    set(oneValueArgs "NAME;DIR")
    set(multiValueArgs "FILES")
    cmake_parse_arguments(ARG
                          ""
                          "${oneValueArgs}"
                          "${multiValueArgs}"
                          ${ARGN}
    )
    add_executable(
        ${ARG_NAME}
        "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_DIR}/${ARG_NAME}.cpp"
        ${ARG_FILES}
    )
    target_include_directories(
        ${ARG_NAME}
        PUBLIC
        "${YAMAL_INC_DIR}"
        "${YAMAL_PYTHON_INC_DIR}"
        "${YAMAL_SRC_DIR}"
    )
    target_link_libraries(
        ${ARG_NAME}
        PUBLIC
        fmc++
        ytp
        PkgConfig::GTEST
        PkgConfig::PYTHON
    )
    add_test(NAME ${ARG_NAME} COMMAND ${ARG_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

yamal_unit_test(
    NAME
    config
    DIR
    fmc++
)
yamal_unit_test(
    NAME
    variant_map
    DIR
    fmc++
)
