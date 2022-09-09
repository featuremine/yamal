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

add_library(
    testcomponent
    SHARED
    "testcomponent.c"
)
target_link_libraries(
    testcomponent
    PRIVATE
    fmc
)
set_target_properties(
    testcomponent
    PROPERTIES PREFIX ""
)
add_library(
    iocomponent
    SHARED
    "iocomponent.c"
)
target_link_libraries(
    iocomponent
    PRIVATE
    fmc
)
set_target_properties(
    iocomponent
    PROPERTIES PREFIX ""
)
add_library(
    shutdowncomponent
    SHARED
    "shutdowncomponent.c"
)
target_link_libraries(
    shutdowncomponent
    PRIVATE
    fmc
)
set_target_properties(
    shutdowncomponent
    PROPERTIES PREFIX ""
)
add_executable(
    tests_fmc_component
    "component.cpp"
)
target_link_libraries(
    tests_fmc_component
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(
    NAME fmc_component
    COMMAND tests_fmc_component "$<TARGET_FILE:testcomponent>"
)

add_executable(
    tests_fmc_config_c
    "config.cpp"
)
target_link_libraries(
    tests_fmc_config_c
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(NAME fmc_config_c COMMAND tests_fmc_config_c)

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
    tests_fmc_extension
    "extension.cpp"
)
target_link_libraries(
    tests_fmc_extension
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(
    NAME fmc_extension
    COMMAND tests_fmc_extension "$<TARGET_FILE:testcomponent>"
)

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

add_executable(
    tests_fmc_memory
    "memory.cpp"
)
target_link_libraries(
    tests_fmc_memory
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(NAME fmc_memory COMMAND tests_fmc_memory)

add_executable(
    tests_fmc_priority_queue
    "priority_queue.cpp"
)
target_link_libraries(
    tests_fmc_priority_queue
    PRIVATE
    fmc
    PkgConfig::GTEST
)
add_test(NAME fmc_priority_queue COMMAND tests_fmc_priority_queue)

if(BUILD_TOOLS)
    add_test(
        NAME fmc_yamal-run
        COMMAND yamal-run -c ../../../tests/fmc/testcomponent.ini -s main -m testcomponent -o testcomponentsched
    )
    set_property(TEST fmc_yamal-run PROPERTY ENVIRONMENT "YAMALCOMPPATH=./tests/fmc")
    add_test(
        NAME fmc_yamal-run_only_cfg
        COMMAND yamal-run -c ../../../tests/fmc/testcomponent_only_cfg.ini -s main
    )
    set_property(TEST fmc_yamal-run_only_cfg PROPERTY ENVIRONMENT "YAMALCOMPPATH=./tests/fmc")
    add_test(
        NAME fmc_yamal-run_array_cfg
        COMMAND yamal-run -c ../../../tests/fmc/testcomponent_array_cfg.ini -s main
    )
    set_property(TEST fmc_yamal-run_array_cfg PROPERTY ENVIRONMENT "YAMALCOMPPATH=./tests/fmc")
endif()
