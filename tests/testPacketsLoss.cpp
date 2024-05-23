//
// Created by Serge on 20.05.2024.
//

#include <gtest/gtest.h>
#include <iostream>
#include "protocol/Session.h"
#include "protocol/Connection.h"
#include "server/STIPServer.h"
#include "client/STIPClient.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <thread>
#include <random>
#include "protocol/errors/STIP_errors.h"
#include <utility>

using namespace std;
using namespace STIP;

using boost::asio::ip::udp;

#define DEF_PROXY_PORT 12226
#define DEF_SERVER_PORT 12223
#define DEF_CLIENT_PORT 12224

//constexpr int BUFFER_SIZE = 4096;

#include "util/UdpProxy.h"

TEST(TestGroupName, Subtest_1) {
    ASSERT_TRUE(1 == 1);
}


TEST(Protocol, MessageTransferingWithProxyDUALSide) {
    boost::asio::io_context io_contextProxy;

    udp::endpoint server_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_SERVER_PORT);
    udp::endpoint client_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_CLIENT_PORT);
    udp::endpoint proxy_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_PROXY_PORT);

    UdpProxy proxy(DEF_CLIENT_PORT, DEF_SERVER_PORT, DEF_PROXY_PORT);
//    proxy.setDelay(300);
    proxy.async_start();

    // sleep 4 seconds

    // subthread for server

    std::string test_string = "Hello, I'm Ilya";

    boost::asio::io_context io_context_server;

    udp::socket socket_server(io_context_server, server_endpoint);
    STIPServer server(socket_server);

    std::vector<thread> threadsProcessors;
    std::thread serverThread([&server, &test_string, &threadsProcessors] {
        for (;;) {
            Connection *serverconnection = server.acceptConnection();
            if (serverconnection == nullptr) break;
            std::cout << "Connection accepted\n\n" << std::endl;

            // processing should be in separate thread
            threadsProcessors.emplace_back([&serverconnection, &test_string] {
                ReceiveMessageSession *received = serverconnection->receiveMessage();
                std::string receivedMessage = received->getDataAsString();
                std::cout << "[SERVER THREAD] Received message: " << receivedMessage << endl;
                ASSERT_EQ(receivedMessage, test_string);
                ASSERT_TRUE(serverconnection->sendMessage(receivedMessage));
                std::cout << "[SERVER THREAD] Message sent: " << receivedMessage << endl;
            });
        }
        cout << "Server thread finished" << endl;
    });

    // sleep 200ms
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // client thread

    boost::asio::io_context io_context;

//    udp::resolver resolver(io_context);
//    int port = DEF_PROXY_PORT;
//    udp::endpoint server_endpoint = *resolver.resolve(udp::v4(), "localhost", to_string(port));

    udp::socket socket(io_context);
    socket.open(udp::v4());
//    udp::endpoint local_endpoint(udp::v4(), DEF_CLIENT_PORT);
    socket.bind(client_endpoint);

    STIPClient client(socket);
    client.startListen();

    Connection *connection = client.connect(proxy_endpoint);
    std::cout << "Connection accepted\n\n" << std::endl;

    ASSERT_TRUE(connection->sendMessage(test_string));

    cout << "\n\nMessage sent\n\n" << endl;

    ReceiveMessageSession *received = connection->receiveMessage();

    ASSERT_EQ(received->getDataAsString(), test_string);
    cout << "[MAIN THREAD] Received message: " << received->getDataAsString() << endl;


    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (auto &th : threadsProcessors) th.join();

    socket_server.cancel();
    serverThread.join();

    socket_server.close();
    client.stopListen();

    proxy.async_stop();
}

// Drop 2 data packets
TEST(Protocol, MessageTransferingWithProxyDropDataPackets) {
    boost::asio::io_context io_contextProxy;

    udp::endpoint server_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_SERVER_PORT);
    udp::endpoint client_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_CLIENT_PORT);
    udp::endpoint proxy_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_PROXY_PORT);

    UdpProxy proxy(DEF_CLIENT_PORT, DEF_SERVER_PORT, DEF_PROXY_PORT);
    proxy.runTest_1(200);

//    proxy.setDelay(300);
    proxy.async_start();


    // subthread for server

    std::string test_string = "Hello, I'm Ilya";

    boost::asio::io_context io_context_server;

    udp::socket socket_server(io_context_server, server_endpoint);
    STIPServer server(socket_server);

    std::vector<thread> threadsProcessors;
    std::thread serverThread([&server, &test_string, &threadsProcessors] {
        for (;;) {
            Connection *serverconnection = server.acceptConnection();
            if (serverconnection == nullptr) break;
            std::cout << "Connection accepted\n\n" << std::endl;

            // processing should be in separate thread
            threadsProcessors.emplace_back([&serverconnection, &test_string] {
                ReceiveMessageSession *received = serverconnection->receiveMessage();
                std::string receivedMessage = received->getDataAsString();
                std::cout << "[SERVER THREAD] Received message: " << receivedMessage << endl;
                ASSERT_EQ(receivedMessage, test_string);
            });
        }

        cout << "Server thread finished" << endl;
    });

    // sleep 200ms
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // client thread

    boost::asio::io_context io_context;



    udp::socket socket(io_context);
    socket.open(udp::v4());
    socket.bind(client_endpoint);

    STIPClient client(socket);
    client.startListen();

    Connection *connection = client.connect(proxy_endpoint);
    std::cout << "Connection accepted\n\n" << std::endl;

    ASSERT_TRUE(connection->sendMessage(test_string));

    cout << "\n\nMessage sent\n\n" << endl;

    for (auto &th : threadsProcessors) th.join();

    socket_server.cancel();
    serverThread.join();

    socket_server.close();
    client.stopListen();
    proxy.async_stop();
}

TEST(Protocol, MessageTransferingWithProxyDropAprovePackets) {
    boost::asio::io_context io_contextProxy;

    udp::endpoint server_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_SERVER_PORT);
    udp::endpoint client_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_CLIENT_PORT);
    udp::endpoint proxy_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_PROXY_PORT);

    UdpProxy proxy(DEF_CLIENT_PORT, DEF_SERVER_PORT, DEF_PROXY_PORT);
//    proxy.setDelay(300);
    proxy.runTest_2(2);
    // sleep 200 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    proxy.async_start();

    // subthread for server

    std::string test_string = "Hello, I'm Ilya";

    boost::asio::io_context io_context_server;

    udp::socket socket_server(io_context_server, server_endpoint);
    STIPServer server(socket_server);

    std::vector<thread> threadsProcessors;
    std::thread serverThread([&server, &test_string, &threadsProcessors] {

        for (;;) {
            Connection *serverconnection = server.acceptConnection();
            if (serverconnection == nullptr) break;
            std::cout << "Connection accepted\n\n" << std::endl;

            // processing should be in separate thread
            threadsProcessors.emplace_back([&serverconnection, &test_string] {
                ReceiveMessageSession *received = serverconnection->receiveMessage();
                std::string receivedMessage = received->getDataAsString();
                std::cout << "[SERVER THREAD] Received message: " << receivedMessage << endl;
                ASSERT_EQ(receivedMessage, test_string);
            });
        }
        for (auto &th : threadsProcessors) th.join();

        cout << "Server thread finished" << endl;
    });



    // sleep 200ms
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // client thread

    boost::asio::io_context io_context;



    udp::socket socket(io_context);
    socket.open(udp::v4());
    socket.bind(client_endpoint);

    STIPClient client(socket);
    client.startListen();

    Connection *connection = client.connect(proxy_endpoint);
    std::cout << "Connection accepted\n\n" << std::endl;

    ASSERT_TRUE(connection->sendMessage(test_string));

    cout << "\n\nMessage sent\n\n" << endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    socket_server.cancel();
    serverThread.join();
    socket_server.close();

    client.stopListen();

    proxy.async_stop();
}

TEST(Protocol, MessageTransferingWithProxyDropInitPackets) {
    boost::asio::io_context io_contextProxy;

    udp::endpoint server_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_SERVER_PORT);
    udp::endpoint client_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_CLIENT_PORT);
    udp::endpoint proxy_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_PROXY_PORT);

    UdpProxy proxy(DEF_CLIENT_PORT, DEF_SERVER_PORT, DEF_PROXY_PORT);
    proxy.setDelay(300);
    proxy.runTest_3(2);
    // sleep 200 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    proxy.async_start();

    // subthread for server

    std::string test_string = "Hello, I'm Ilya";

    boost::asio::io_context io_context_server;

    udp::socket socket_server(io_context_server, server_endpoint);
    STIPServer server(socket_server);

    std::vector<thread> threadsProcessors;
    std::thread serverThread([&server, &test_string, &threadsProcessors] {

        for (;;) {
            Connection *serverconnection = server.acceptConnection();
            if (serverconnection == nullptr) break;
            std::cout << "Connection accepted\n\n" << std::endl;

            // processing should be in separate thread
            threadsProcessors.emplace_back([&serverconnection, &test_string] {
                ReceiveMessageSession *received = serverconnection->receiveMessage();
                std::string receivedMessage = received->getDataAsString();
                std::cout << "[SERVER THREAD] Received message: " << receivedMessage << endl;
                ASSERT_EQ(receivedMessage, test_string);
            });
        }
        for (auto &th : threadsProcessors) th.join();

        cout << "Server thread finished" << endl;
    });



    // sleep 200ms
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // client thread

    boost::asio::io_context io_context;



    udp::socket socket(io_context);
    socket.open(udp::v4());
    socket.bind(client_endpoint);

    STIPClient client(socket);
    client.startListen();

    Connection *connection = client.connect(proxy_endpoint);
    std::cout << "Connection accepted\n\n" << std::endl;

    ASSERT_TRUE(connection->sendMessage(test_string));

    cout << "\n\nMessage sent\n\n" << endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    socket_server.cancel();
    serverThread.join();
    socket_server.close();

    client.stopListen();

    proxy.async_stop();
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
//    ::testing::InitGoogleMock(&argc, argv);

    return RUN_ALL_TESTS();
}