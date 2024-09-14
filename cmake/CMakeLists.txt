include(${CMAKE_CURRENT_SOURCE_DIR}/external_option.cmake)
set(TARGET_NAME ${USER_TARGET_PREFIX})

include(${CMAKE_CURRENT_SOURCE_DIR}/internal_option.cmake)

# 库文件的目标路径
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib64)

# 头文件的目标路径
# install 时仅拷贝 include 目录内的文件与子目录，不拷贝 include 目录本身
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION
    ${CMAKE_INSTALL_PREFIX}/include PATTERN "internal" EXCLUDE)
