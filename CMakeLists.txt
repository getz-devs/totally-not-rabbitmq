cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(rabbit)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)          # Enable c++14 standard


# ---- Add googletest directly ----
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


add_executable(rabbitServerL2
        src/server/STIPServer.cpp
        examples/server.cpp
        src/protocol/Connection.cpp
        src/protocol/Session.cpp
)

add_executable(rabbitClientL2
        src/client/STIPClient.cpp
        examples/client.cpp
        src/protocol/Connection.cpp
        src/protocol/Session.cpp
)

add_executable(rabbitServerTest
        src/server/STIPServer.cpp
        src/client/STIPClient.cpp
        tests/testStipSend.cpp
        src/protocol/Connection.cpp
        src/protocol/Session.cpp
)

# add sources dir
include_directories(src)

target_compile_features(rabbitServerL2 PRIVATE cxx_std_17)
target_compile_features(rabbitClientL2 PRIVATE cxx_std_17)


# ---- Add Boost via CPM ----

file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.7/CPM.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

CPMAddPackage(
        NAME Boost
        VERSION 1.84.0
        URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
        URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
        OPTIONS "BOOST_ENABLE_CMAKE ON" "BOOST_INCLUDE_LIBRARIES container\\\;asio" # Note the escapes!
)

target_link_libraries(rabbitServerL2 PRIVATE
        Boost::asio ws2_32
)
target_link_libraries(rabbitClientL2 PRIVATE
        Boost::asio ws2_32
)

# ---- Create test executable ----
target_link_libraries(rabbitServerTest
        GTest::gtest_main
        Boost::asio ws2_32
)
include(GoogleTest)
gtest_discover_tests(rabbitServerTest)

