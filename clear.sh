#!/bin/bash

# 进入脚本所在目录（可选，根据实际需要）
# cd "$(dirname "$0")"

# 设置目标目录
TARGET_DIR="data/kvdict"

# 检查目录是否存在
if [ -d "$TARGET_DIR" ]; then
    echo "Deleting all files in $TARGET_DIR ..."
    rm -f "$TARGET_DIR"/*
    echo "Done."
else
    echo "Directory $TARGET_DIR does not exist."
    exit 1
fi
