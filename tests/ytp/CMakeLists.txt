#[===[
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
]===]

function(add_ytp_test NAME)
    add_executable(
        tests_ytp_${NAME}
        "${NAME}.cpp"
    )
    target_include_directories(
        tests_ytp_${NAME}
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/ytp"
    )
    target_link_libraries(
        tests_ytp_${NAME}
        PUBLIC
        ytp
        gtest
    )
    add_test(NAME ytp_${NAME} COMMAND tests_ytp_${NAME})
endfunction()

add_ytp_test("yamal")
add_ytp_test("time")
add_ytp_test("timeline")
add_ytp_test("sequence")
add_ytp_test("stream")

add_executable(
    tests_ytp_compiles_c
    "compiles.c"
)
target_link_libraries(
    tests_ytp_compiles_c
    PRIVATE
    ytp
)
add_test(NAME ytp_compiles_c COMMAND tests_ytp_compiles_c)