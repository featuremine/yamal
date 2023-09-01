#[===[
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
]===]

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
        "${YAMAL_SRC_DIR}"
    )
    target_link_libraries(
        ${ARG_NAME}
        PUBLIC
        fmc++
        ytp
        py_yamal_dep
        gtest
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
