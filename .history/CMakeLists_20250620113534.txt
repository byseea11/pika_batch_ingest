cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(bingest)
set(CMAKE_BUILD_TYPE Debug)

# 设置 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 如果编译器版本小于 9，链接 stdc++fs
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
    target_link_libraries(bingest stdc++fs)
endif()

# 设置源文件
file(GLOB SOURCES "src/*.cpp")

# 添加可执行文件
add_executable(bingest ${SOURCES})

# 包含头文件目录
include_directories(include)
