#[===[
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
]===]

add_executable(
    tests_fmc_config
    "config.cpp"
)
target_link_libraries(
    tests_fmc_config
    PRIVATE
    ytp
    fmc++
    gtest
)
add_test(NAME fmc_config COMMAND tests_fmc_config)

add_executable(
    tests_fmc_strings
    "strings.cpp"
)
target_link_libraries(
    tests_fmc_strings
    PRIVATE
    ytp
    fmc++
    gtest
)
add_test(NAME fmc_strings COMMAND tests_fmc_strings)

add_executable(
    tests_fmc_decimal128
    "decimal128.cpp"
)
target_link_libraries(
    tests_fmc_decimal128
    PRIVATE
    ytp
    fmc++
    gtest
)
add_test(NAME fmc_decimal128 COMMAND tests_fmc_decimal128)

add_executable(
    tests_fmc_counters
    "counters.cpp"
)
target_link_libraries(
    tests_fmc_counters
    PRIVATE
    fmc++
    ytp
    gtest
)
add_test(NAME fmc_counters COMMAND tests_fmc_counters)


add_executable(
    tests_fmc_fxpt128
    "fxpt128.cpp"
)
target_link_libraries(
    tests_fmc_fxpt128
    PRIVATE
    ytp
    fmc++
    gtest
)
add_test(NAME fmc_fxpt128 COMMAND tests_fmc_fxpt128)

add_executable(
    tests_fmc_errorpp
    "error.cpp"
)
target_link_libraries(
    tests_fmc_errorpp
    PRIVATE
    ytp
    fmc++
    gtest
)
add_test(NAME fmc_errorpp COMMAND tests_fmc_errorpp)

add_executable(
    tests_fmc_rprice
    "rprice.cpp"
)
target_link_libraries(
    tests_fmc_rprice
    PRIVATE
    ytp
    fmc++
    gtest
)
add_test(NAME fmc_rprice COMMAND tests_fmc_rprice)

add_executable(
    tests_fmc_rational64
    "rational64.cpp"
)
target_link_libraries(
    tests_fmc_rational64
    PRIVATE
    ytp
    fmc++
    gtest
)
add_test(NAME fmc_rational64 COMMAND tests_fmc_rational64)

add_executable(
    tests_fmc_containers
    "containers.cpp"
)
target_link_libraries(
    tests_fmc_containers
    PRIVATE
    ytp
    fmc++
    gtest
)
add_test(NAME fmc_containers COMMAND tests_fmc_containers)
