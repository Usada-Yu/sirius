# 外部配置选项

# compile
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9")
        message(WARNING
            "current gnuc version: ${CMAKE_CXX_COMPILER_VERSION}\n"
            "gnuc version must be at least 4.9\n"
            "please edit cmake cache or rebuild")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.1")
        message(WARNING
            "current clang version: ${CMAKE_CXX_COMPILER_VERSION}\n"
            "clang version must be at least 3.1\n"
            "please edit cmake cache or rebuild")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.8")
        message(WARNING
            "current msvc version: ${CMAKE_CXX_COMPILER_VERSION}\n"
            "msvc version must be at least 16.8\n"
            "please edit cmake cache or rebuild")
    endif()
else()
    message(FATAL_ERROR
        "${CMAKE_CXX_COMPILER_ID} is not supported")
endif()

# library type: static or shared
option(BUILD_SHARED_LIBS "build shared libraries" OFF)

# asan enable
option(USER_ASAN "asan enable" OFF)

# gcov enable
option(USER_GCOV "gcov enable" OFF)

# target prefix
set(USER_TARGET_PREFIX "sirius" CACHE STRING "target name")
