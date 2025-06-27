#!/bin/bash

# 默认参数值
DEFAULT_SIZE="10M"
DEFAULT_DICT="kvdict"

# 解析命令行参数
while getopts ":n:d:" opt; do
  case $opt in
    n)
      size="$OPTARG"
      ;;
    d)
      dict="$OPTARG"
      ;;
    \?)
      echo "无效选项: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "选项 -$OPTARG 需要参数值." >&2
      exit 1
      ;;
  esac
done

# 使用默认值（如果用户未输入）
size=${size:-$DEFAULT_SIZE}
dict=${dict:-$DEFAULT_DICT}

# 执行构建和运行命令
./build.sh
cd build
./mock -n "$size" -d "$dict"