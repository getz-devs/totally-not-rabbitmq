# src/protocol/CMakeLists.txt

set(PROTOCOL_SOURCE_FILES
        Connection.cpp
        Session.cpp
        SessionKiller.cpp
)

add_library(STIPProtocol STATIC ${PROTOCOL_SOURCE_FILES})
target_include_directories(STIPProtocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(STIPProtocol PUBLIC Boost::asio ws2_32)
