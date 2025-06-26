#!/bin/bash

# 默认测试可执行文件名
TEST_EXECUTABLE=./build/test_bingest

# 检查是否传入了测试过滤器参数
if [ $# -eq 0 ]; then
    echo "Usage: ./test.sh <TestFilter>"
    echo "Example: ./test.sh DataGenTest.*"
    exit 1
fi

echo "➡️ 运行 build.sh 构建项目..."
./build.sh

# 获取测试名过滤器
FILTER=$1

echo "✅ 构建完成，开始运行测试: ${FILTER:-[全部]}"
# 执行测试
$TEST_EXECUTABLE --gtest_filter=$FILTER
