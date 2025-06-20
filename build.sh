#!/bin/bash

# Navigate to the directory containing the CMakeLists.txt file
cd /home/byseea/code/opensum/pika_batch_ingest || exit

# Create a build directory if it doesn't exist
mkdir -p build
cd build || exit

# Run CMake to configure the project
cmake ..

# Build the project
make