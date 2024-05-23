# tests/CMakeLists.txt

# Define test executables
add_executable(STIPTest testStipSend.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/server/STIPServer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/client/STIPClient.cpp)
add_executable(STIPTestProxy testPacketsLoss.cpp util/UdpProxy.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/server/STIPServer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/client/STIPClient.cpp)
add_executable(STIPTestBigData testBigData.cpp util/UdpProxy.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/server/STIPServer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/client/STIPClient.cpp)

# Link test executables with GoogleTest
target_link_libraries(STIPTest PRIVATE GTest::gtest_main STIPProtocol Boost::asio ws2_32)
target_link_libraries(STIPTestProxy PRIVATE GTest::gtest_main STIPProtocol Boost::asio ws2_32)
target_link_libraries(STIPTestBigData PRIVATE GTest::gtest_main STIPProtocol Boost::asio ws2_32)

# Enable GoogleTest discover
include(GoogleTest)
gtest_discover_tests(STIPTest STIPTestProxy STIPTestBigData)
