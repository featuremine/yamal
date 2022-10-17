find_package(PkgConfig)
pkg_check_modules(GTEST REQUIRED IMPORTED_TARGET gtest)

add_subdirectory(fmc)
add_subdirectory(fmc++)
add_subdirectory(ytp)
add_subdirectory(decnumber)
