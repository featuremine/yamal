add_executable(
    tests_fmc_compiles_c
    "compiles.c"
)
target_link_libraries(
    tests_fmc_compiles_c
    PRIVATE
    ytp
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
    ytp
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
    ytp
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
    ytp
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
    ytp
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
    ytp
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
    ytp
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
    ytp
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
    ytp
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
    ytp
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
    ytp
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
    ytp
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
    ytp
    PkgConfig::GTEST
)
add_test(NAME fmc_priority_queue COMMAND tests_fmc_priority_queue)

if(BUILD_TOOLS)
    add_test(
        NAME fmc_yamal-run_ini
        COMMAND yamal-run -c ${CMAKE_CURRENT_SOURCE_DIR}/testcomponent.ini -s main -m testcomponent -o testcomponentsched
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_property(TEST fmc_yamal-run_ini PROPERTY ENVIRONMENT "YAMALCOMPPATH=${CMAKE_CURRENT_BINARY_DIR}")
    add_test(
        NAME fmc_yamal-run_ini_only_cfg
        COMMAND yamal-run -c ${CMAKE_CURRENT_SOURCE_DIR}/testcomponent_only_cfg.ini -s main
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_property(TEST fmc_yamal-run_ini_only_cfg PROPERTY ENVIRONMENT "YAMALCOMPPATH=${CMAKE_CURRENT_BINARY_DIR}")
    add_test(
        NAME fmc_yamal-run_ini_array_cfg
        COMMAND yamal-run -c ${CMAKE_CURRENT_SOURCE_DIR}/testcomponent_array_cfg.ini -s main
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_property(TEST fmc_yamal-run_ini_array_cfg PROPERTY ENVIRONMENT "YAMALCOMPPATH=${CMAKE_CURRENT_BINARY_DIR}")

    add_test(
        NAME fmc_yamal-run_json
        COMMAND yamal-run -c ${CMAKE_CURRENT_SOURCE_DIR}/testcomponent.json -m testcomponent -o testcomponentsched -j
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_property(TEST fmc_yamal-run_json PROPERTY ENVIRONMENT "YAMALCOMPPATH=${CMAKE_CURRENT_BINARY_DIR}")
    add_test(
        NAME fmc_yamal-run_json_array_cfg
        COMMAND yamal-run -c ${CMAKE_CURRENT_SOURCE_DIR}/testcomponent_array_cfg.json -j
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_property(TEST fmc_yamal-run_json_array_cfg PROPERTY ENVIRONMENT "YAMALCOMPPATH=${CMAKE_CURRENT_BINARY_DIR}")

    if (BUILD_WHEEL)
        add_test(
            NAME fmc_yamal-run.py_json
            COMMAND ${PYTHON3_BIN} ${WHEEL_yamal_SCRIPTS}/build/scripts/yamal-run.py --config ${CMAKE_CURRENT_SOURCE_DIR}/testcomponent.json --module testcomponent --component testcomponentsched
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        set_property(TEST fmc_yamal-run.py_json PROPERTY ENVIRONMENT "YAMALCOMPPATH=${CMAKE_CURRENT_BINARY_DIR}")
        set_property(TEST fmc_yamal-run.py_json PROPERTY ENVIRONMENT "PATH=${WHEEL_yamal_SCRIPTS}")
        set_property(TEST fmc_yamal-run.py_json PROPERTY ENVIRONMENT "PYTHONPATH=${WHEEL_yamal_BUILD_DIR}/build/lib")
        add_test(
            NAME fmc_yamal-run.py_json_array_cfg
            COMMAND ${PYTHON3_BIN} ${WHEEL_yamal_SCRIPTS}/build/scripts/yamal-run.py --config ${CMAKE_CURRENT_SOURCE_DIR}/testcomponent_array_cfg.json
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        set_property(TEST fmc_yamal-run.py_json_array_cfg PROPERTY ENVIRONMENT "YAMALCOMPPATH=${CMAKE_CURRENT_BINARY_DIR}")
        set_property(TEST fmc_yamal-run.py_json_array_cfg PROPERTY ENVIRONMENT "PYTHONPATH=${WHEEL_yamal_BUILD_DIR}/build/lib")
    endif()
endif()
