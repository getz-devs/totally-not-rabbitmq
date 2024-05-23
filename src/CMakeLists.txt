# src/CMakeLists.txt

# Add subdirectory for protocol
add_subdirectory(protocol)
include_directories(DataModel)
# Source files
set(SOURCE_FILES
        server/STIPServer.cpp
        client/STIPClient.cpp
        services/TaskService/TaskService.cpp
        services/UserDBService/UserDBService.cpp
        services/TaskQueue/TaskQueue.h
)

# Function to create executable targets
function(add_rabbit_executable target_name)
    add_executable(${target_name} ${ARGN} ${SOURCE_FILES})
    target_compile_features(${target_name} PRIVATE cxx_std_17)
    target_link_libraries(${target_name} PUBLIC
            STIPProtocol
            argparse::argparse
            Boost::asio
            ws2_32
            nlohmann_json::nlohmann_json
    )
endfunction()

# Define executables
add_rabbit_executable(rabbitServerL2Example ${CMAKE_CURRENT_SOURCE_DIR}/../examples/server.cpp)
add_rabbit_executable(rabbitClientL2Example ${CMAKE_CURRENT_SOURCE_DIR}/../examples/client.cpp)

add_rabbit_executable(rabbitServerL3cmd cmd/serverMain.cpp rabbitCore/server/RabbitServer.cpp)
add_rabbit_executable(rabbitClientL3cmd cmd/clientMain.cpp rabbitCore/client/RabbitClient.cpp)
add_rabbit_executable(rabbitWorkerL3cmd cmd/workerMain.cpp rabbitCore/worker/RabbitWorker.cpp)
