include(${CMAKE_CURRENT_SOURCE_DIR}/option.cmake)

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

set(_src_dir ${PROJECT_SOURCE_DIR}/src)

include_directories(${PROJECT_SOURCE_DIR}/include)

aux_source_directory(${_src_dir} _src_list)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
if(BUILD_SHARED_LIBS)
    add_library(${USER_TARGET_NAME} SHARED ${_src_list})
else()
    add_library(${USER_TARGET_NAME} STATIC ${_src_list})
endif()
