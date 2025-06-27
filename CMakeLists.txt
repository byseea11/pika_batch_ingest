cmake_minimum_required(VERSION 3.10)
project(bingest)

# 构建类型与标准
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件路径
include_directories(${CMAKE_SOURCE_DIR}/include test third/ThreadPool)

# -----------------------------------------------------------------------------
# Fetch nlohmann/json (Header-only)
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# -----------------------------------------------------------------------------
# 禁用rocksdb的测试和工具
set(WITH_TESTS OFF CACHE BOOL "Disable RocksDB tests" FORCE)
set(WITH_GFLAGS OFF CACHE BOOL "Disable GFLAGS" FORCE)
set(WITH_GTEST OFF CACHE BOOL "Disable RocksDB internal gtest" FORCE)
set(WITH_BENCHMARK_TOOLS OFF CACHE BOOL "Disable benchmark tools" FORCE)
set(WITH_TOOLS OFF CACHE BOOL "Disable RocksDB tools" FORCE)
set(ROCKSDB_BUILD_TESTS OFF CACHE BOOL "Disable building RocksDB tests" FORCE)

# -----------------------------------------------------------------------------
# 添加子模块依赖
add_subdirectory(third/googletest)
add_subdirectory(third/rocksdb)

# -----------------------------------------------------------------------------
# 排除文件列表构建正则表达式
set(EXCLUDE_FILES "mock.cpp" "exchange.cpp")
set(EXCLUDE_REGEX "")
foreach(file ${EXCLUDE_FILES})
    list(APPEND EXCLUDE_REGEX ".*/${file}$")
endforeach()
string(REPLACE ";" "|" EXCLUDE_REGEX "${EXCLUDE_REGEX}")

# -----------------------------------------------------------------------------
# 查找源文件
file(GLOB_RECURSE ALL_SOURCES "src/**/*.cpp")
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

# 构建测试目标源文件（排除 mock & exchange）
set(TESTABLE_SOURCES ${ALL_SOURCES})
list(FILTER TESTABLE_SOURCES EXCLUDE REGEX ${EXCLUDE_REGEX})
add_executable(test_bingest ${TEST_SOURCES} ${TESTABLE_SOURCES})
target_link_libraries(test_bingest
    PRIVATE
    gmock_main
    nlohmann_json::nlohmann_json
    rocksdb
)
target_compile_definitions(test_bingest PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")

# -----------------------------------------------------------------------------
# Mock 可执行文件（排除 exchange）
set(MOCK_SOURCES ${ALL_SOURCES})
list(FILTER MOCK_SOURCES EXCLUDE REGEX ".*/exchange.cpp$")
add_executable(mock ${MOCK_SOURCES})
target_link_libraries(mock
    PRIVATE
    nlohmann_json::nlohmann_json
)
target_compile_definitions(mock PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")

# -----------------------------------------------------------------------------
# Exchange 可执行文件（排除 mock）
set(EXCHANGE_SOURCES ${ALL_SOURCES})
list(FILTER EXCHANGE_SOURCES EXCLUDE REGEX ".*/mock.cpp$")
add_executable(exchange ${EXCHANGE_SOURCES})
target_link_libraries(exchange
    PRIVATE
    nlohmann_json::nlohmann_json
    rocksdb
)
target_compile_definitions(exchange PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")

# -----------------------------------------------------------------------------
# 若 GCC 版本 < 9，手动链接 stdc++fs
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
    target_link_libraries(mock PRIVATE stdc++fs)
    target_link_libraries(exchange PRIVATE stdc++fs)
endif()

# -----------------------------------------------------------------------------
# 启用 CTest 测试框架
enable_testing()
add_test(NAME test_bingest COMMAND test_bingest)
