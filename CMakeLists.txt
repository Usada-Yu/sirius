#[[ ex.
cmake -Dplat=hi3519dv500 -H. -Bbuild

cmake --build ./build --target help
cmake --build ./build
cmake --build ./build --target all      [default]
cmake --build ./build --target clean
cmake --build ./build --target edit_cache

cmake --build ./build --target install
]]
cmake_minimum_required(VERSION 3.5)

project(
    PROJECT-SIRIUS
    VERSION "1.0.0"
    DESCRIPTION "Base library encapsulation"
    LANGUAGES "C" "CXX"
    HOMEPAGE_URL "https://github.com/Usada-Yu/sirius"
)

if (WIN32)
    message(FATAL_ERROR "This project only supports linux")
endif()


# platform: (x86|hi3519dv500|sd3403v100|win)
if(NOT plat)
    message(WARNING "Use the default platform: x86")
    set(plat "x86")
endif()

if(plat STREQUAL "x86" OR plat STREQUAL "win")
elseif(plat STREQUAL "hi3519dv500")
    set(_cross aarch64-linux-gnu-hi3519dv500-v2- CACHE INTERNAL "")
elseif(plat STREQUAL "sd3403v100")
    set(_cross aarch64-himix210-linux-sd3403v100-v1- CACHE INTERNAL "")
else()
    message(FATAL_ERROR "Invalid platform: ${plat}")
endif()

set(CMAKE_C_COMPILER "${_cross}gcc")
set(CMAKE_CXX_COMPILER "${_cross}g++")
message(STATUS "Building platform: ${plat}")

add_subdirectory(cmake)

add_definitions(-DLOG_MODULE_NAME="lib-sirius")

# 库文件的目标路径
# 通过make edit_cache中的CMAKE_INSTALL_PREFIX选项配置
install(TARGETS ${USER_TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

# 头文件的目标路径
# make install时仅拷贝include目录内的文件与子目录，不拷贝include目录本身
# 通过make edit_cache中的CMAKE_INSTALL_INCLUDEDIR选项配置
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR} PATTERN "internal" EXCLUDE)
