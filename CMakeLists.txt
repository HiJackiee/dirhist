# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(dirhist VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)
add_executable(dirhist src/main.cpp src/snapshot.cpp src/util.cpp)
target_include_directories(dirhist PRIVATE include)
target_link_libraries(dirhist PRIVATE OpenSSL::Crypto)