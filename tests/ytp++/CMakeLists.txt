#[===[
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
]===]

function(add_ytppp_test NAME)
    add_executable(
        tests_ytppp_${NAME}
        "${NAME}.cpp"
    )
    target_link_libraries(
        tests_ytppp_${NAME}
        PUBLIC
        ytp
        gtest
    )
    add_test(NAME ytppp_${NAME} COMMAND tests_ytppp_${NAME})
endfunction()

add_ytppp_test("yamal")
