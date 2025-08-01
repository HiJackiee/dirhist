cmake_minimum_required(VERSION 3.15)
project(dirhist VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 开启优化等级
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# 查找动态链接的 OpenSSL
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

add_executable(dirhist
    src/main.cpp
    src/snapshot.cpp
    src/serialize.cpp
    src/log.cpp
    src/diff.cpp
    src/util.cpp
    src/cli.cpp
)

target_include_directories(dirhist PRIVATE include)
target_link_libraries(dirhist PRIVATE OpenSSL::Crypto pthread dl)