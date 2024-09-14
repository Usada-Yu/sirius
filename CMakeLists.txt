cmake_minimum_required(VERSION 3.5)

project(
    PROJECT-SIRIUS
    VERSION "1.0.0"
    DESCRIPTION "Base library encapsulation"
    LANGUAGES "C" "CXX"
    HOMEPAGE_URL "https://github.com/Usada-Yu/sirius"
)


option(USER_GTEST_ENABLE "google test enable" OFF)

# not yet
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(FATAL_ERROR "this project only supports linux")
endif()

add_subdirectory(cmake)

if (USER_GTEST_ENABLE)
    enable_testing()
    add_subdirectory(unittests)
endif()
