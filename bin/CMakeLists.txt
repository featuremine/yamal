add_executable(
    ytp-perf
    "${PROJECT_SOURCE_DIR}/bin/ytp_perf.cpp"
)
target_include_directories(
    ytp-perf
    PUBLIC
    "${TCLAP_INCLUDEDIR}"
    PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
)
target_link_libraries(
    ytp-perf
    PUBLIC
    ytp
)

add_executable(
    ytp-stats
    "${PROJECT_SOURCE_DIR}/bin/ytp_stats.cpp"
)
target_include_directories(
    ytp-stats
    PUBLIC
    "${TCLAP_INCLUDEDIR}"
    PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
)
target_link_libraries(
    ytp-stats
    PUBLIC
    ytp
)

# add_executable(
#     ytp-daemon
#     "${PROJECT_SOURCE_DIR}/bin/ytp-daemon.cpp"
# )
# target_include_directories(
#     ytp-daemon
#     PUBLIC
#     "${TCLAP_INCLUDEDIR}"
#     PRIVATE
#     "${PROJECT_SOURCE_DIR}/src"
# )
# target_link_libraries(
#     ytp-daemon
#     PUBLIC
#     ytp
# )