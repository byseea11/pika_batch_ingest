#!/bin/bash

# 当前脚本目录（即 googletest/）
cd "$(dirname "$0")"

# 目标目录：存放 samples 目录的路径
SAMPLES_DIR="./googletest/samples"

# 输出目录
BUILD_DIR="./build_sample"
mkdir -p "$BUILD_DIR"

# 编译设置
CXX=g++
CXXFLAGS="-I ${SAMPLES_DIR} -lgtest -lgtest_main -lpthread"

# 查找 samples 目录中的所有 *_unittest.cc 文件
FILES=(${SAMPLES_DIR}/*_unittest.cc)

# 如果没找到文件则退出
if [ ${#FILES[@]} -eq 0 ]; then
    echo "❌ No *_unittest.cc files found in $SAMPLES_DIR"
    exit 1
fi

# 编译循环
for test_path in "${FILES[@]}"; do
    test_file=$(basename "$test_path")
    base_name=$(basename "$test_path" _unittest.cc)
    source_file="${SAMPLES_DIR}/${base_name}.cc"
    exe_name="${BUILD_DIR}/${base_name}"

    if [[ -f "$source_file" ]]; then
        echo "📦 Compiling $source_file + $test_file -> $exe_name"
        $CXX "$source_file" "$test_path" -o "$exe_name" $CXXFLAGS
    else
        echo "📦 Compiling $test_file only -> $exe_name"
        $CXX "$test_path" -o "$exe_name" $CXXFLAGS
    fi

    if [[ $? -ne 0 ]]; then
        echo "❌ Failed to compile $base_name"
    else
        echo "✅ Built $exe_name"
    fi
done
