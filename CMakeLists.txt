cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(rabbit)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)          # Enable c++14 standard

#### Dependencies ###
#add_subdirectory(dependencies)


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


add_executable(rabbitServerL2Example
        src/server/STIPServer.cpp
        examples/server.cpp
        src/protocol/Connection.cpp
        src/protocol/Session.cpp
)

add_executable(rabbitClientL2Example
        src/client/STIPClient.cpp
        examples/client.cpp
        src/protocol/Connection.cpp
        src/protocol/Session.cpp
)

# -- L3

add_executable(rabbitServerL3cmd
        src/cmd/serverMain.cpp
        src/server/STIPServer.cpp
        src/rabbitCore/server/RabbitServer.cpp
        src/protocol/Connection.cpp
        src/protocol/Session.cpp
        src/services/TaskService/TaskService.cpp
        src/services/UserDBService/UserDBService.cpp
        src/protocol/SessionKiller.cpp
        src/services/TaskQueue/TaskQueue.h
)

add_executable(rabbitClientL3cmd
        src/cmd/clientMain.cpp
        src/client/STIPClient.cpp
        src/rabbitCore/client/RabbitClient.cpp
        src/protocol/Connection.cpp
        src/protocol/Session.cpp
        src/protocol/SessionKiller.cpp
        src/services/TaskQueue/TaskQueue.h
)

add_executable(rabbitWorkerL3cmd
        src/cmd/workerMain.cpp
        src/client/STIPClient.cpp
        src/rabbitCore/worker/RabbitWorker.cpp
        src/protocol/Connection.cpp
        src/protocol/Session.cpp
        src/protocol/SessionKiller.cpp
        src/services/TaskQueue/TaskQueue.h
)

# -- Tests

add_executable(STIPTest
        src/server/STIPServer.cpp
        src/client/STIPClient.cpp
        tests/testStipSend.cpp
        src/protocol/Connection.cpp
        src/protocol/Session.cpp
        src/protocol/SessionKiller.cpp
)

# add sources dir
include_directories(src)
include_directories(src/DataModel)

target_compile_features(rabbitServerL2Example PRIVATE cxx_std_17)
target_compile_features(rabbitClientL2Example PRIVATE cxx_std_17)


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

find_package(SqliteOrm)
find_package(argparse)
find_package(nlohmann_json)

#find_package(asio REQUIRED)

target_link_libraries(rabbitServerL2Example PRIVATE
        Boost::asio ws2_32
#        asio::asio
)
target_link_libraries(rabbitClientL2Example PRIVATE
        Boost::asio ws2_32
#        asio::asio
)

# -- L3

target_link_libraries(rabbitServerL3cmd PRIVATE
        Boost::asio ws2_32
        argparse::argparse
        sqlite_orm::sqlite_orm
        nlohmann_json::nlohmann_json
)

target_link_libraries(rabbitClientL3cmd PRIVATE
        Boost::asio ws2_32
        argparse::argparse
        nlohmann_json::nlohmann_json
)

target_link_libraries(rabbitWorkerL3cmd PRIVATE
        Boost::asio ws2_32
        argparse::argparse
        nlohmann_json::nlohmann_json
)

# ---- Create test executable ----

target_link_libraries(STIPTest
        GTest::gtest_main
        Boost::asio ws2_32
)
include(GoogleTest)
gtest_discover_tests(STIPTest)

