add_executable(
    tests_fmc_compiles_c
    "compiles.c"
)
target_link_libraries(
    tests_fmc_compiles_c
    PRIVATE
    fmc
)
add_test(NAME fmc_compiles_c COMMAND tests_fmc_compiles_c)

add_executable(
    tests_fmc_error
    "error.cpp"
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
    "files.cpp"
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
    "math.cpp"
)
target_link_libraries(
    tests_fmc_math
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(NAME fmc_math COMMAND tests_fmc_math)

add_executable(
    tests_fmc_time
    "time.cpp"
)
target_link_libraries(
    tests_fmc_time
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(NAME fmc_time COMMAND tests_fmc_time)
