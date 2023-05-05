add_executable(
    tests_ytp_yamal
    "yamal.cpp"
)
target_link_libraries(
    tests_ytp_yamal
    PUBLIC
    ytp
    PkgConfig::GTEST
)
add_test(NAME ytp_yamal COMMAND tests_ytp_yamal)

add_executable(
    tests_ytp_time
    "time.cpp"
)
target_link_libraries(
    tests_ytp_time
    PRIVATE
    ytp
    PkgConfig::GTEST
)
add_test(NAME ytp_time COMMAND tests_ytp_time)

add_executable(
    tests_ytp_control
    "control.cpp"
)
target_link_libraries(
    tests_ytp_control
    PRIVATE
    ytp
    PkgConfig::GTEST
)
add_test(NAME ytp_control COMMAND tests_ytp_control)

add_executable(
    tests_ytp_timeline
    "timeline.cpp"
)
target_link_libraries(
    tests_ytp_timeline
    PRIVATE
    ytp
    PkgConfig::GTEST
)
add_test(NAME ytp_timeline COMMAND tests_ytp_timeline)

add_executable(
    tests_ytp_sequence
    "sequence.cpp"
)
target_link_libraries(
    tests_ytp_sequence
    PRIVATE
    ytp
    PkgConfig::GTEST
)
add_test(NAME ytp_sequence COMMAND tests_ytp_sequence)

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
