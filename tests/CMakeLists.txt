add_executable(
    tests_ytp_yamal
    "${PROJECT_SOURCE_DIR}/tests/yamal.cpp"
)
target_include_directories(
    tests_ytp_yamal
    PRIVATE
    "${PROJECT_SOURCE_DIR}/tests"
)
target_link_libraries(
    tests_ytp_yamal
    PRIVATE
    ytp
    PkgConfig::GTEST
)
add_test(NAME ytp_yamal COMMAND tests_ytp_yamal)

add_executable(
    tests_ytp_peer
    "${PROJECT_SOURCE_DIR}/tests/peer.cpp"
)
target_include_directories(
    tests_ytp_peer
    PRIVATE
    "${PROJECT_SOURCE_DIR}/tests"
)
target_link_libraries(
    tests_ytp_peer
    PRIVATE
    ytp
    PkgConfig::GTEST
)
add_test(NAME ytp_peer COMMAND tests_ytp_peer)

add_executable(
    tests_ytp_channel
    "${PROJECT_SOURCE_DIR}/tests/channel.cpp"
)
target_include_directories(
    tests_ytp_channel
    PRIVATE
    "${PROJECT_SOURCE_DIR}/tests"
)
target_link_libraries(
    tests_ytp_channel
    PRIVATE
    ytp
    PkgConfig::GTEST
)
add_test(NAME ytp_channel COMMAND tests_ytp_channel)

add_executable(
    tests_ytp_time
    "${PROJECT_SOURCE_DIR}/tests/time.cpp"
)
target_include_directories(
    tests_ytp_time
    PRIVATE
    "${PROJECT_SOURCE_DIR}/tests"
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
    "${PROJECT_SOURCE_DIR}/tests/control.cpp"
)
target_include_directories(
    tests_ytp_control
    PRIVATE
    "${PROJECT_SOURCE_DIR}/tests"
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
    "${PROJECT_SOURCE_DIR}/tests/timeline.cpp"
)
target_include_directories(
    tests_ytp_timeline
    PRIVATE
    "${PROJECT_SOURCE_DIR}/tests"
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
    "${PROJECT_SOURCE_DIR}/tests/sequence.cpp"
)
target_include_directories(
    tests_ytp_sequence
    PRIVATE
    "${PROJECT_SOURCE_DIR}/tests"
)
target_link_libraries(
    tests_ytp_sequence
    PRIVATE
    ytp
    PkgConfig::GTEST
)
add_test(NAME ytp_sequence COMMAND tests_ytp_sequence)

add_executable(
    tests_ytp_containers
    "${PROJECT_SOURCE_DIR}/tests/containers.cpp"
)
target_include_directories(
    tests_ytp_containers
    PRIVATE
    "${PROJECT_SOURCE_DIR}/tests"
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
    "${PROJECT_SOURCE_DIR}/tests/compiles.c"
)
target_include_directories(
    tests_ytp_compiles_c
    PRIVATE
    "${PROJECT_SOURCE_DIR}/tests"
)
target_link_libraries(
    tests_ytp_compiles_c
    PRIVATE
    ytp
)
add_test(NAME ytp_compiles_c COMMAND tests_ytp_compiles_c)

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/tests/data")
file(COPY "${PROJECT_SOURCE_DIR}/tests/data/synth_book.ore" DESTINATION "${PROJECT_BINARY_DIR}/tests/data/")
