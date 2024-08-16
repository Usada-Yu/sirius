option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
set(CMAKE_INSTALL_INCLUDEDIR "${PROJECT_SOURCE_DIR}/artifact/include" CACHE STRING "Default include directory")

option(USER_ASAN "Default asan option" OFF)
option(USER_GCOV "Default gcov option" OFF)
set(USER_TARGET_NAME "sirius" CACHE STRING "Default target name")
