#[===[
        COPYRIGHT (c) 2019-2024 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
]===]

function(add_tools_test NAME)
    add_executable(
        tests_tools_${NAME}
        "${NAME}.cpp"
    )
    target_include_directories(
        tests_tools_${NAME}
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/tools"
    )
    target_link_libraries(
        tests_tools_${NAME}
        PUBLIC
        ytp
        PkgConfig::GTEST
    )
    add_test(NAME tools_${NAME} COMMAND tests_tools_${NAME})
endfunction()

add_tools_test("daemon")
