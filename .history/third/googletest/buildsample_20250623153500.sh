#!/bin/bash
cd "$(dirname "$0")"
BUILD_DIR="./build_sample"
mkdir -p "$BUILD_DIR"

CXX=g++
CXXFLAGS="-I ./ -lgtest -lgtest_main -lpthread"

for test_file in *_unittest.cc; do
    base_name=$(basename "$test_file" _unittest.cc)
    source_file="${base_name}.cc"
    exe_name="${BUILD_DIR}/${base_name}"

    if [[ -f "$source_file" ]]; then
        echo "Compiling $source_file + $test_file -> $exe_name"
        $CXX "$source_file" "$test_file" -o "$exe_name" $CXXFLAGS
    else
        echo "Compiling $test_file only -> $exe_name"
        $CXX "$test_file" -o "$exe_name" $CXXFLAGS
    fi

    if [[ $? -ne 0 ]]; then
        echo "❌ Failed to compile $base_name"
    else
        echo "✅ Built $exe_name"
    fi
done
