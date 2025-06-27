#!/bin/bash

# 添加下载.gitmodules的命令

# Navigate to the directory containing the CMakeLists.txt file 将地址改为当前项目地址，不使用绝对地址
cd /home/byseea/code/opensum/pika_batch_ingest || exit

# Create a build directory if it doesn't exist
mkdir -p build
cd build || exit

# Run CMake to configure the project
cmake ..

# Build the project
make -j4 && make install