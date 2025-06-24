#!/bin/bash

# 当前脚本目录（即 googletest/）
cd "$(dirname "$0")"

# 目标目录：samples 源代码路径
SAMPLES_DIR="./googletest/samples"

# 可执行文件输出目录
BUILD_DIR="./build_sample"
mkdir -p "$BUILD_DIR"

# 编译器设置
CXX=g++
CXXFLAGS="-I ${SAMPLES_DIR} -lgtest -lgtest_main -lpthread"

# 查找所有 *_unittest.cc 测试文件
FILES=(${SAMPLES_DIR}/*_unittest.cc)

# 没有文件就报错退出
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

    # 处理 sample5 的特殊情况
    if [[ "$test_file" == "sample5_unittest.cc" ]]; then
        echo "📦 Compiling $test_file + sample1.cc -> $exe_name"
        $CXX "$SAMPLES_DIR/sample1.cc" "$test_path" -o "$exe_name" $CXXFLAGS

    elif [[ -f "$source_file" ]]; then
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
