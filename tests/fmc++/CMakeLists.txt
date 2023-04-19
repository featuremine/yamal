add_executable(
    tests_fmc_config
    "config.cpp"
)
target_link_libraries(
    tests_fmc_config
    PRIVATE
    ytp
    fmc++
    PkgConfig::GTEST
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
    PkgConfig::GTEST
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
    PkgConfig::GTEST
)
add_test(NAME fmc_decimal128 COMMAND tests_fmc_decimal128)

add_executable(
    tests_fmc_errorpp
    "error.cpp"
)
target_link_libraries(
    tests_fmc_errorpp
    PRIVATE
    ytp
    fmc++
    PkgConfig::GTEST
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
    PkgConfig::GTEST
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
    PkgConfig::GTEST
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
    PkgConfig::GTEST
)
add_test(NAME fmc_containers COMMAND tests_fmc_containers)
