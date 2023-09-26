#[===[
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
]===]

function(add_ytppp_test NAME)
    add_executable(
        tests_ytp_${NAME}pp
        "${NAME}.cpp"
    )
    target_link_libraries(
        tests_ytp_${NAME}pp
        PUBLIC
        ytp
        gtest
    )
    add_test(NAME ytp_${NAME}pp COMMAND tests_ytp_${NAME}pp)
endfunction()

add_ytppp_test("yamal")
