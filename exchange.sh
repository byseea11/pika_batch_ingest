#!/bin/bash

# 默认参数
DEFAULT_KV="kvdict/data_1.json"
DEFAULT_SST="sst/data_1.sst"

# 解析命令行参数
while getopts ":k:s:" opt; do
  case $opt in
    k)
      kv="$OPTARG"
      ;;
    s)
      sst="$OPTARG"
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

# 使用默认值
kv=${kv:-$DEFAULT_KV}
sst=${sst:-$DEFAULT_SST}

echo "使用 kv: $kv"
echo "使用 sst: $sst"

# 构建
./build.sh

# 进入build目录并运行
cd build
./exchange -k "$kv" -s "$sst"
