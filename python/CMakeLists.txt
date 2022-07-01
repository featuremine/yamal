macro(getAllLibraries iTarget iLibsReturnValue iIncsReturnValue)
    if(NOT TARGET ${iTarget})
        list(APPEND ${iLibsReturnValue} "-l${iTarget}")
    else()
        set(path $<TARGET_FILE:${iTarget}>)
        if(NOT ${path} IN_LIST ${iLibsReturnValue})
            list(APPEND ${iLibsReturnValue} ${path})
        endif()

        get_target_property(includes ${iTarget} INTERFACE_INCLUDE_DIRECTORIES)
        FOREACH(include ${includes})
            if(NOT ${include} IN_LIST ${iIncsReturnValue})
                list(APPEND ${iIncsReturnValue} ${include})
            endif()
        ENDFOREACH()

        get_target_property(linkedLibraries ${iTarget} INTERFACE_LINK_LIBRARIES)
        if(NOT "${linkedLibraries}" STREQUAL "")
            FOREACH(linkedLibrary ${linkedLibraries})
                getAllLibraries(${linkedLibrary} ${iLibsReturnValue} ${iIncsReturnValue})
            ENDFOREACH()
        endif()
    endif()
endmacro()


find_package(PkgConfig)
find_program(PYTHON3_BIN "python3" REQUIRED)
pkg_check_modules(PYTHON3 REQUIRED IMPORTED_TARGET python3-embed)

getAllLibraries(ytp YTP_PACKAGE_LIBS_LIST YTP_PACKAGE_INCLUDES_LIST)
list(APPEND YTP_PACKAGE_INCLUDES_LIST "${CMAKE_CURRENT_SOURCE_DIR}/include")
list(JOIN YTP_PACKAGE_INCLUDES_LIST ":" YTP_PACKAGE_INCLUDES)
list(JOIN YTP_PACKAGE_LIBS_LIST ":" YTP_PACKAGE_LIBS)

add_custom_target(
    ytp_py

    ${CMAKE_COMMAND} -E env
    "CC=${CMAKE_C_COMPILER}"
    "CXX=${CMAKE_CXX_COMPILER}"
    "PACKAGE_VERSION=${PROJECT_VERSION}"
    "YTP_PACKAGE_LIBS=${YTP_PACKAGE_LIBS}"
    "YTP_PACKAGE_INCLUDES=${YTP_PACKAGE_INCLUDES}"

    "${PYTHON3_BIN}" "setup.py"
    "build"
    "--build-base=${CMAKE_CURRENT_BINARY_DIR}/build"

    "egg_info"
    "--egg-base" "${CMAKE_CURRENT_BINARY_DIR}"

    "bdist_wheel"
    "--bdist-dir=${CMAKE_CURRENT_BINARY_DIR}/bdist"
    "--dist-dir=${CMAKE_CURRENT_BINARY_DIR}/dist"

    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
