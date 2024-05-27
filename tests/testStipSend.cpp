#include <gtest/gtest.h>
#include <iostream>

#include "protocol/STIP.h"
#include "protocol/Session.h"
#include "protocol/Connection.h"
#include "server/STIPServer.h"
#include "client/STIPClient.h"
#include "protocol/STIPVersion.h"


#include <gtest/gtest.h>

#include "protocol/errors/STIP_errors.h"

using namespace std;
using namespace STIP;

TEST(TestGroupName, Subtest_1) {
    ASSERT_TRUE(1 == 1);
}


TEST(Protocol, PingTest) {
    // subthread for server

    boost::asio::io_context io_context_server;

    udp::socket socket_server(io_context_server, udp::endpoint(udp::v4(), 12222));
    STIPServer server(socket_server);

    std::thread serverThread([&server] {
        for (;;) {
            Connection *connection = server.acceptConnection();
            if (connection == nullptr) break;
            std::cout << "Connection accepted\n\n" << std::endl;
        }
        cout << "Server thread finished" << endl;
    });


    // sleep 200ms
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // client thread

    boost::asio::io_context io_context;

    udp::resolver resolver(io_context);
    udp::endpoint server_endpoint = *resolver.resolve(udp::v4(), "localhost", "12222");

    udp::socket socket(io_context);
    socket.open(udp::v4());

    STIPClient client(socket);
    client.startListen();

    Connection *connection = client.connect(server_endpoint);
    std::cout << "Connection accepted\n\n" << std::endl;

    uint32_t result = connection->pingVersion();
    ASSERT_EQ(result, STIP_PROTOCOL_VERSION);


    socket_server.cancel();
    serverThread.join();
    client.stopListen();
}

TEST(Protocol, MessageTransfering) {
    // subthread for server

    std::string test_string = "Hello, I'm Ilya";

    boost::asio::io_context io_context_server;

    udp::socket socket_server(io_context_server, udp::endpoint(udp::v4(), 12223));
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

    udp::resolver resolver(io_context);
    udp::endpoint server_endpoint = *resolver.resolve(udp::v4(), "localhost", "12223");

    udp::socket socket(io_context);
    socket.open(udp::v4());

    STIPClient client(socket);
    client.startListen();

    Connection *connection = client.connect(server_endpoint);
    std::cout << "Connection accepted\n\n" << std::endl;

    ASSERT_TRUE(connection->sendMessage(test_string));

    cout << "\n\nMessage sent\n\n" << endl;

    ReceiveMessageSession *received = connection->receiveMessage();

    ASSERT_EQ(received->getDataAsString(), test_string);
    cout << "[MAIN THREAD] Received message: " << received->getDataAsString() << endl;


    // sleep
    for (auto &th : threadsProcessors) th.join();

    socket_server.cancel();
    serverThread.join();
    socket_server.close();

    client.stopListen();
}

TEST(Protocol, CatchException) {
    // sleep

    // subthread for server

    std::string test_string = "Hello, I'm Ilya";

    boost::asio::io_context io_context_server;

    udp::socket socket_server(io_context_server, udp::endpoint(udp::v4(), 12227));
    STIPServer server(socket_server);


    // client thread

    boost::asio::io_context io_context;

    udp::resolver resolver(io_context);
    udp::endpoint server_endpoint = *resolver.resolve(udp::v4(), "localhost", "12227");

    udp::socket socket(io_context);
    socket.open(udp::v4());

    STIPClient client(socket);
    client.startListen();


    EXPECT_THROW(static_cast<void>(client.connect(server_endpoint)), STIP::errors::STIPTimeoutException);



    client.stopListen();
    socket_server.close();

//    serverThread.join();
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
//    ::testing::InitGoogleMock(&argc, argv);

    return RUN_ALL_TESTS();
}