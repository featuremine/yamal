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
    tests_fmc_shared_map
    "shared_map.cpp"
)
target_link_libraries(
    tests_fmc_shared_map
    PRIVATE
    ytp
    PkgConfig::GTEST
)
add_test(NAME fmc_shared_map COMMAND tests_fmc_shared_map)

add_executable(
    tests_fmc_threaded
    "threaded.cpp"
)
target_link_libraries(
    tests_fmc_threaded
    PRIVATE
    ytp
    PkgConfig::GTEST
)
add_test(NAME fmc_threaded COMMAND tests_fmc_threaded)

add_executable(
    tests_fmc_variant_map
    "variant_map.cpp"
)
target_link_libraries(
    tests_fmc_variant_map
    PRIVATE
    ytp
    fmc++
    PkgConfig::GTEST
)
add_test(NAME fmc_variant_map COMMAND tests_fmc_variant_map)

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
    tests_fmc_static_vector
    "static_vector.cpp"
)
target_link_libraries(
    tests_fmc_static_vector
    PRIVATE
    ytp
    fmc++
    PkgConfig::GTEST
)
add_test(NAME fmc_static_vector COMMAND tests_fmc_static_vector)
