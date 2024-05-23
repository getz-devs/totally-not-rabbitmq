cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(rabbit)

set(CMAKE_CXX_STANDARD 17)

# Dependencies
include(FetchContent)

# Fetch GoogleTest
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add Boost via CPM
file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.7/CPM.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

CPMAddPackage(
        NAME Boost
        VERSION 1.84.0
        URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
        URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
        OPTIONS "BOOST_ENABLE_CMAKE ON" "BOOST_INCLUDE_LIBRARIES container\\\;asio" # Note the escapes!
)

find_package(SqliteOrm REQUIRED)
find_package(argparse REQUIRED)

# Include directories
include_directories(src)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)