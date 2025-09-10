# 设置交叉编译
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 设置工具链路径
set(TOOLCHAIN_DIR /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf)

# 设置编译器
set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++)

# 设置其他工具
set(CMAKE_AR ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-ar)
set(CMAKE_NM ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-nm)
set(CMAKE_OBJCOPY ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-objdump)
set(CMAKE_RANLIB ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-ranlib)
set(CMAKE_STRIP ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-strip)

# 设置目标系统的根目录
# 注意：这里可能需要根据您的工具链实际结构进行调整
# SYSROOT 是工具链目录下的libc不是简单的工具链路径这里要分清楚
set(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/arm-linux-gnueabihf/libc)
set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/arm-linux-gnueabihf/libc)

# 设置查找策略
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# 设置编译器标志
set(CMAKE_C_FLAGS "--sysroot=${TOOLCHAIN_DIR}/arm-linux-gnueabihf/libc -march=armv7-a -mfpu=neon -mfloat-abi=hard" CACHE STRING "C compiler flags")
set(CMAKE_CXX_FLAGS "--sysroot=${TOOLCHAIN_DIR}/arm-linux-gnueabihf/libc -march=armv7-a -mfpu=neon -mfloat-abi=hard" CACHE STRING "C++ compiler flags")