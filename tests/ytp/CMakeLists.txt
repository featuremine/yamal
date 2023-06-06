function(add_ytp_test NAME)
    add_executable(
        tests_ytp_${NAME}
        "${NAME}.cpp"
    )
    target_link_libraries(
        tests_ytp_${NAME}
        PUBLIC
        ytp
        PkgConfig::GTEST
    )
    add_test(NAME ytp_${NAME} COMMAND tests_ytp_${NAME})

    add_executable(
        tests_ytp_inv_${NAME}
        "${NAME}.cpp"
    )
    target_link_libraries(
        tests_ytp_inv_${NAME}
        PUBLIC
        ytp-inv-endianess
        PkgConfig::GTEST
    )
    add_test(NAME ytp_inv_${NAME} COMMAND tests_ytp_inv_${NAME})
endfunction()

add_ytp_test("yamal")
add_ytp_test("peer")
add_ytp_test("channel")
add_ytp_test("time")
add_ytp_test("control")
add_ytp_test("timeline")
add_ytp_test("sequence")

add_executable(
    tests_ytp_containers
    "containers.cpp"
)
target_link_libraries(
    tests_ytp_containers
    PRIVATE
    ytp
    PkgConfig::GTEST
)
add_test(NAME ytp_containers COMMAND tests_ytp_containers)

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