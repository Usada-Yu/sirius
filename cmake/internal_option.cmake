# 内部选项

add_definitions(-DLOG_MODULE_NAME="lib-sirius")
add_definitions(-DLOG_PRNT_BUF_SIZE=1024)

if(USER_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address)
    add_link_options(-fsanitize=address)
endif()

if(USER_GCOV)
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Werror -fPIC)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

set(_src_dir ${PROJECT_SOURCE_DIR}/src)

aux_source_directory(${_src_dir} _src_list)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
if(BUILD_SHARED_LIBS)
    add_library(${TARGET_NAME} SHARED ${_src_list})
else()
    add_library(${TARGET_NAME} STATIC ${_src_list})
endif()
