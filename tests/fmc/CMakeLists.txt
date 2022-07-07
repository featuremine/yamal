add_executable(
    tests_fmc_compiles_c
    "${PROJECT_SOURCE_DIR}/tests/compiles.c"
)
target_link_libraries(
    tests_fmc_compiles_c
    PRIVATE
    fmc
)
add_test(NAME fmc_compiles_c COMMAND tests_fmc_compiles_c)

add_executable(
    tests_fmc_error
    "${PROJECT_SOURCE_DIR}/tests/error.cpp"
)
target_link_libraries(
    tests_fmc_error
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(NAME fmc_error COMMAND tests_fmc_error)

add_executable(
    tests_fmc_files
    "${PROJECT_SOURCE_DIR}/tests/files.cpp"
)
target_link_libraries(
    tests_fmc_files
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(NAME fmc_files COMMAND tests_fmc_files)

add_executable(
    tests_fmc_math
    "${PROJECT_SOURCE_DIR}/tests/math.cpp"
)
target_link_libraries(
    tests_fmc_math
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(NAME fmc_math COMMAND tests_fmc_math)

add_executable(
    tests_fmc_shared_map
    "${PROJECT_SOURCE_DIR}/tests/shared_map.cpp"
)
target_link_libraries(
    tests_fmc_shared_map
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(NAME fmc_shared_map COMMAND tests_fmc_shared_map)

add_executable(
    tests_fmc_threaded
    "${PROJECT_SOURCE_DIR}/tests/threaded.cpp"
)
target_link_libraries(
    tests_fmc_threaded
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(NAME fmc_threaded COMMAND tests_fmc_threaded)

add_executable(
    tests_fmc_time
    "${PROJECT_SOURCE_DIR}/tests/time.cpp"
)
target_link_libraries(
    tests_fmc_time
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(NAME fmc_time COMMAND tests_fmc_time)

add_executable(
    tests_fmc_variant_map
    "${PROJECT_SOURCE_DIR}/tests/config/variant_map.cpp"
)
target_link_libraries(
    tests_fmc_variant_map
    PRIVATE
    fmc
    fmc++
    PkgConfig::GTEST
)
add_test(NAME fmc_variant_map COMMAND tests_fmc_variant_map)
