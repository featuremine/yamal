find_program(SPHINX_EXECUTABLE NAMES sphinx-build)
set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(SPHINX_BUILD ${PROJECT_BINARY_DIR}/python/docs/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)
add_custom_command(
    OUTPUT ${SPHINX_INDEX_FILE} ${SPHINX_BUILD}

    COMMAND
    ${SPHINX_EXECUTABLE} -b html
    "${SPHINX_SOURCE}"
    "${SPHINX_BUILD}"

    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/python/docs"

    DEPENDS
    # Other docs files you want to track should go here (or in some variable)
    ${SPHINX_SOURCE}/index.rst
    
    MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
    
    COMMENT "Generating documentation with Sphinx"
)

# Nice named target so we can run the job easily
add_custom_target(ytp-python-docs ALL DEPENDS ${SPHINX_INDEX_FILE})
