# 设置最低CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(bingest)

# 设置构建类型为 Debug
set(CMAKE_BUILD_TYPE Debug)

# 设置 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义需要排除的文件列表
set(EXCLUDE_FILES "mock.cpp" "exchange.cpp")

# 生成排除的正则表达式
set(EXCLUDE_REGEX "")
foreach(file ${EXCLUDE_FILES})
    list(APPEND EXCLUDE_REGEX ".*/${file}$")
endforeach()

# 将排除的正则表达式连接成一个合并的正则表达式
string(REPLACE ";" "|" EXCLUDE_REGEX "${EXCLUDE_REGEX}")

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

# 查找所有源文件
file(GLOB_RECURSE ALL_SOURCES "src/**/*.cpp")
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

# 用于测试目标的源文件（排除 mock|exchange.cpp）
set(TESTABLE_SOURCES ${ALL_SOURCES})
list(FILTER TESTABLE_SOURCES EXCLUDE REGEX ${EXCLUDE_REGEX})

# 构建测试程序（排除 mock|exchange.cpp）
add_executable(test_bingest ${TEST_SOURCES} ${TESTABLE_SOURCES})
target_link_libraries(test_bingest
    PRIVATE
    gmock_main
    nlohmann_json::nlohmann_json
)

# Mock 可执行文件
set(MOCK_SOURCES ${ALL_SOURCES})
list(FILTER MOCK_SOURCES EXCLUDE REGEX ".*/exchange.cpp$")  # 排除 exchange.cpp
add_executable(mock ${MOCK_SOURCES})
target_link_libraries(mock
    nlohmann_json::nlohmann_json
)

# Exchange 可执行文件
set(EXCHANGE_SOURCES ${ALL_SOURCES})
list(FILTER EXCHANGE_SOURCES EXCLUDE REGEX ".*/mock.cpp$")  # 排除 mock.cpp
add_executable(exchange ${EXCHANGE_SOURCES})
target_link_libraries(exchange
    nlohmann_json::nlohmann_json
)

# 主目标和测试目标均需定义
target_compile_definitions(mock PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(exchange PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(test_bingest PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")

# 若低版本 GCC，手动链接 stdc++fs
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
    target_link_libraries(mock PRIVATE stdc++fs)
    target_link_libraries(exchange PRIVATE stdc++fs)
endif()

# 启用测试支持
enable_testing()
add_test(NAME test_bingest COMMAND test_bingest)