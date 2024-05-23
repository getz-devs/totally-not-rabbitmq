//
// Created by Serge on 20.05.2024.
//

#include <gtest/gtest.h>
#include <iostream>

#include "protocol/STIP.h"
#include "protocol/Session.h"
#include "protocol/Connection.h"
#include "server/STIPServer.h"
#include "client/STIPClient.h"
#include "protocol/STIPVersion.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <thread>
#include <random>

#include "protocol/errors/STIP_errors.h"
#include <boost/bind/bind.hpp>
#include <utility>

using namespace std;
using namespace STIP;

using boost::asio::ip::udp;

#define DEF_PROXY_PORT 12226
#define DEF_SERVER_PORT 12223
#define DEF_CLIENT_PORT 12224

constexpr int BUFFER_SIZE = 4096;

class UdpProxy {
public:
    UdpProxy(int client_port, int server_port, int proxy_port)
            : io_context_(), socket_(io_context_), client_port_(client_port), server_port_(server_port),
              proxy_port_(proxy_port) {}

    void start() {
        is_running_ = true;
        try {
            // Создание UDP сокета для прокси сервера
            socket_.open(udp::v4());
            socket_.bind(udp::endpoint(udp::v4(), proxy_port_));

            // Адрес клиента и сервера
            client_endpoint_ = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), client_port_);
            server_endpoint_ = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), server_port_);

            // Основной цикл прокси сервера
            while (true) {
                udp::endpoint remote_endpoint;
                boost::system::error_code error;
                std::size_t len = socket_.receive_from(boost::asio::buffer(recv_buffer_), remote_endpoint, 0, error);
                if (!is_running_) break;
                if (error && error != boost::asio::error::message_size) {
                    std::cerr << "Receive failed: " << error.message() << std::endl;
                    continue;
                }

                // delay
                if (delay_ > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay_));

                // drop packet
                std::uniform_real_distribution<float> distribution(0.0, 100.0);

                float random_number = distribution(generator);
                if (random_number < loss_percentage_) {
                    continue;
                }

                if (test_1_skip_command__3) {
                    if (test_skip_counter < test_skip_target_count) {
                        // read command to int
                        int command = *(int *) &recv_buffer_[0];
                        std::cout << "[Proxy] Skip command: " << command << std::endl;
                        if (command == 3) {
                            test_skip_counter++;
                            continue;
                        }
                    } else {
                        test_1_skip_command__3 = false;
                    }
                }

                if (test_2_skip_command__4) {
                    if (test_skip_counter < test_skip_target_count) {
                        // read command to int
                        int command = *(int *) &recv_buffer_[0];
                        std::cout << "[Proxy] Skip command: " << command << std::endl;
                        if (command == 4) {
                            test_skip_counter++;
                            continue;
                        }
                    } else {
                        test_2_skip_command__4 = false;
                    }
                }

                // Определение направления и пересылка данных
                if (remote_endpoint == client_endpoint_) {
                    // Данные от клиента к серверу
                    socket_.send_to(boost::asio::buffer(recv_buffer_, len), server_endpoint_, 0, error);
                    if (error) {
                        std::cerr << "Send to server failed: " << error.message() << std::endl;
                    }
                } else if (remote_endpoint == server_endpoint_) {
                    // Данные от сервера к клиенту
                    socket_.send_to(boost::asio::buffer(recv_buffer_, len), client_endpoint_, 0, error);
                    if (error) {
                        std::cerr << "Send to client failed: " << error.message() << std::endl;
                    }
                } else {
                    std::cerr << "Unknown sender: " << remote_endpoint << std::endl;
                }
            }
        } catch (std::exception &e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }

    void async_start() {
        if (is_running_) return;
        is_running_ = true;

        async_thread_ = std::thread([this] { start(); });
    }

    void async_stop() {
        if (!is_running_) return;
        is_running_ = false;
        socket_.close();
        if (async_thread_.joinable()) {
            async_thread_.join();
        }
    }

    void setDelay(int delay) {
        delay_ = delay;
    }

    void setLossPercentage(float loss_percentage) {
        loss_percentage_ = loss_percentage;
    }

    void runTest_1(int target_count) {
        test_1_skip_command__3 = true;
        test_skip_counter = 0;
        test_skip_target_count = target_count;
    }

    void stopTest_1() {
        test_1_skip_command__3 = false;
        test_skip_counter = 0;
        test_skip_target_count = 0;
    }

    void runTest_2(int target_count) {
        test_2_skip_command__4 = true;
        test_skip_counter = 0;
        test_skip_target_count = target_count;
    }

    void stopTest_2() {
        test_2_skip_command__4 = false;
        test_skip_counter = 0;
        test_skip_target_count = 0;
    }

    ~UdpProxy() {
        async_stop();
    }
private:
    boost::asio::io_context io_context_;
    udp::socket socket_;
    udp::endpoint client_endpoint_;
    udp::endpoint server_endpoint_;
    boost::array<char, BUFFER_SIZE> recv_buffer_;
    std::thread async_thread_;
    bool is_running_ = false;
    // atomic delay
    std::atomic<int> delay_ = 0;
    std::atomic<float> loss_percentage_ = 0;
    std::default_random_engine generator;
    int client_port_;
    int server_port_;
    int proxy_port_;
    bool test_1_skip_command__3 = false;
    int test_skip_counter = 0;
    int test_skip_target_count;

    bool test_2_skip_command__4 = false;
};

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
    proxy.runTest_1(2);

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

//    std::this_thread::sleep_for(std::chrono::seconds(1));
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
    proxy.setDelay(300);
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


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
//    ::testing::InitGoogleMock(&argc, argv);

    return RUN_ALL_TESTS();
}