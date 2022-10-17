add_executable(
    tests_decnumber_decimal
    "decimal.cpp"
)
target_link_libraries(
    tests_decnumber_decimal
    PRIVATE
    fmc++
    PkgConfig::GTEST
)
add_test(NAME decnumber_decimal COMMAND tests_decnumber_decimal)
