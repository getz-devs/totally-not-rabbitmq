cmake_minimum_required(VERSION 3.13)  # CMake version check
project(rabbit)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)          # Enable c++14 standard

# packet manager

file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.7/CPM.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

add_executable(rabbit main.cpp)

CPMAddPackage(
        NAME Boost
        VERSION 1.84.0
        URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
        URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
        OPTIONS "BOOST_ENABLE_CMAKE ON" "BOOST_INCLUDE_LIBRARIES asio" # Note the escapes!
)

target_link_libraries(rabbit PRIVATE Boost::asio ws2_32)