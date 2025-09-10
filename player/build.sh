#!/bin/bash

# 设置错误时退出
set -e

# 项目根目录
PROJECT_ROOT=$(cd `dirname $0`; pwd)
echo "项目根目录: $PROJECT_ROOT"

# 构建目录
BUILD_DIR=${PROJECT_ROOT}/build
echo "构建目录: $BUILD_DIR"

# 工具链文件路径
TOOLCHAIN_FILE=${PROJECT_ROOT}/toolchain/arm-linux-gnueabihf.cmake
echo "工具链文件: $TOOLCHAIN_FILE"

# 检查工具链文件是否存在
if [ ! -f "$TOOLCHAIN_FILE" ]; then
    echo "错误: 工具链文件不存在: $TOOLCHAIN_FILE"
    echo "请确保工具链文件路径正确"
    exit 1
fi

# 清理并创建构建目录
echo "清理构建目录..."
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

# 运行CMake，指定工具链文件
echo "运行CMake..."
cmake -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} ..

# 检查CMake是否成功
if [ $? -ne 0 ]; then
    echo "CMake配置失败"
    exit 1
fi

# 编译
echo "开始编译..."
make -j$(nproc)  # 使用所有可用的CPU核心加速编译

# 检查编译是否成功
if [ $? -ne 0 ]; then
    echo "编译失败"
    exit 1
fi

echo "编译完成！"
echo "可执行文件位于: ${PROJECT_ROOT}/bin/"