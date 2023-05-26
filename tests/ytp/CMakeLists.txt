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
set_tests_properties(
    ytp_yamal
    PROPERTIES
    TIMEOUT 60
)

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
set_tests_properties(
    ytp_time
    PROPERTIES
    TIMEOUT 60
)

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
target_include_directories(
    tests_ytp_timeline
    PRIVATE
    "${PROJECT_SOURCE_DIR}/src/ytp"
)
add_test(NAME ytp_timeline COMMAND tests_ytp_timeline)
set_tests_properties(
    ytp_timeline
    PROPERTIES
    TIMEOUT 60
)

add_executable(
    tests_ytp_stream
    "stream.cpp"
)
target_link_libraries(
    tests_ytp_stream
    PRIVATE
    ytp
    PkgConfig::GTEST
)
target_include_directories(
    tests_ytp_stream
    PRIVATE
    "${PROJECT_SOURCE_DIR}/src/ytp"
)
add_test(NAME ytp_stream COMMAND tests_ytp_stream)
set_tests_properties(
    ytp_stream
    PROPERTIES
    TIMEOUT 60
)

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
set_tests_properties(
    ytp_sequence
    PROPERTIES
    TIMEOUT 60
)

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
set_tests_properties(
    ytp_compiles_c
    PROPERTIES
    TIMEOUT 60
)
