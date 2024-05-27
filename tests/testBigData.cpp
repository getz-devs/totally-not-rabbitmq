#include <gtest/gtest.h>
#include <iostream>

#include "protocol/STIP.h"
#include "protocol/Session.h"
#include "protocol/Connection.h"
#include "server/STIPServer.h"
#include "client/STIPClient.h"
#include <boost/asio.hpp>
#include <thread>
#include <random>
#include <fstream>

#include "protocol/errors/STIP_errors.h"


using namespace std;
using namespace STIP;

using boost::asio::ip::udp;

#define DEF_PROXY_PORT 12226
#define DEF_SERVER_PORT 12223
#define DEF_CLIENT_PORT 12224

#include "util/UdpProxy.h"
#define USE_PROXY

#define bigDataLocalPath "C:\\Projects\\suai\\totally-not-rabbitmq\\tests\\voina-i-mir.mobi"

TEST(Protocol, BigMessageTransfering) {
    boost::asio::io_context io_contextProxy;

    udp::endpoint server_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_SERVER_PORT);
    udp::endpoint client_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_CLIENT_PORT);
    udp::endpoint proxy_endpoint = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), DEF_PROXY_PORT);

#ifdef USE_PROXY
    UdpProxy proxy(DEF_CLIENT_PORT, DEF_SERVER_PORT, DEF_PROXY_PORT);
    proxy.setDelay(5);

//    proxy.setLossPercentage(35.0f);
    proxy.setLossPercentage(5.0f);
// sleep 200 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    proxy.async_start();
#endif
    // open file for big data. In result i need void* and size_t
    std::ifstream bigFile;

    void* bigData = nullptr;
    size_t bigDataSize = 0;
    bigFile.open(bigDataLocalPath, std::ios::binary);
    if (bigFile.is_open()) {
        bigFile.seekg(0, std::ios::end);
        bigDataSize = bigFile.tellg();
        bigFile.seekg(0, std::ios::beg);
        bigData = malloc(bigDataSize);
        bigFile.read((char*) bigData, bigDataSize);
        bigFile.close();
    }

    std::cout << "Big data size: " << bigDataSize << " bytes" << std::endl;

//    std::string test_string = "Hello, I'm Ilya";

    boost::asio::io_context io_context_server;

    udp::socket socket_server(io_context_server, server_endpoint);
    STIPServer server(socket_server);

    std::vector<thread> threadsProcessors;
    std::thread serverThread([&server, bigData, &bigDataSize, &threadsProcessors] {

        for (;;) {
            Connection *serverconnection = server.acceptConnection();
            if (serverconnection == nullptr) break;
            std::cout << "Connection accepted\n\n" << std::endl;

            // processing should be in separate thread
            threadsProcessors.emplace_back([&serverconnection, &bigData, &bigDataSize] {
                ReceiveMessageSession *received = serverconnection->receiveMessage();
                std::pair<void*, size_t> receivedMessage = received->getData();

                ASSERT_EQ(receivedMessage.second, bigDataSize);
                ASSERT_EQ(memcmp(receivedMessage.first, bigData, bigDataSize), 0);
            });
        }
        for (auto &th: threadsProcessors) th.join();

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

#ifdef USE_PROXY
    Connection *connection = client.connect(proxy_endpoint);
#else
    Connection *connection = client.connect(server_endpoint);
#endif

    std::cout << "Connection accepted\n\n" << std::endl;

    ASSERT_TRUE(connection->sendMessage(bigData, bigDataSize));

    cout << "\n\nMessage sent\n\n" << endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    socket_server.cancel();
    serverThread.join();
    socket_server.close();

    client.stopListen();
#ifdef USE_PROXY
    proxy.async_stop();
#endif
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
//    ::testing::InitGoogleMock(&argc, argv);

    return RUN_ALL_TESTS();
}