cmake_minimum_required(VERSION 3.10)
project(bingest)
set(CMAKE_BUILD_TYPE Debug)

# 设置 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include test third/ThreadPool)

# 引入 FetchContent 来下载 JSON 库
include(FetchContent)

# 下载 nlohmann/json 库
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)



# 添加 GoogleTest/GoogleMock（使用源码编译方式）
add_subdirectory(third/googletest)

# 查找源文件
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

# 用于测试目标的源文件（排除 mock.cpp）
set(TESTABLE_SOURCES ${ALL_SOURCES})
list(FILTER TESTABLE_SOURCES EXCLUDE REGEX ".*/mock.cpp$")

# 构建主程序：包含 mock.cpp
add_executable(bingest ${ALL_SOURCES})
target_link_libraries(bingest PRIVATE nlohmann_json::nlohmann_json)

# 构建测试程序：排除 mock.cpp
add_executable(test_bingest ${TEST_SOURCES} ${TESTABLE_SOURCES})
target_link_libraries(test_bingest
    PRIVATE
    gmock_main
    nlohmann_json::nlohmann_json
)

# 主目标和测试目标均需定义
target_compile_definitions(bingest PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(test_bingest PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")

# 若低版本 GCC，手动链接 stdc++fs
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
    target_link_libraries(bingest PRIVATE stdc++fs)
endif()

# 启用测试支持
enable_testing()
add_test(NAME test_bingest COMMAND test_bingest)
