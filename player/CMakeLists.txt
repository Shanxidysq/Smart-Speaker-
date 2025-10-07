cmake_minimum_required(VERSION 3.10)
set(TOOLCHAIN_DIR /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf)
# 项目设置 指定语言cpp
project(SampleProject)
set(TARGET threadpool)

# 制定编译出来的arm架构文件路径
set(ALSA_ROOT ${TOOLCHAIN_DIR}/alsa-lib-1.2.7.1/_install)
set(ALSA_INCLUDE_DIR ${ALSA_ROOT}/include)
set(ALSA_LIBRARY   ${ALSA_ROOT}/lib/libasound.so)

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/inc)

# 设置C++11标准
# C++ 14兼容make_unique
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
aux_source_directory(src SRC_LIST)


add_executable(${TARGET} ${SRC_LIST} main.cpp)

target_include_directories(${TARGET} PRIVATE ${ALSA_INCLUDE_DIR})
# 链接pthread库 和 alsa-lib

target_link_libraries(${TARGET} PRIVATE 
                                ${ALSA_LIBRARY}
                                pthread)
# 设置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)