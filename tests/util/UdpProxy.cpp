//
// Created by Serge on 23.05.2024.
//

#include "UdpProxy.h"

#include <iostream>
#include <chrono>

using boost::asio::ip::udp;


UdpProxy::UdpProxy(int client_port, int server_port, int proxy_port)
        : io_context_(), socket_(io_context_), client_port_(client_port), server_port_(server_port), proxy_port_(proxy_port) {
    socket_.open(udp::v4());
    udp::socket::receive_buffer_size bigbufsize(INT_MAX);
    socket_.set_option(bigbufsize);
    socket_.bind(udp::endpoint(udp::v4(), proxy_port_));
}

void UdpProxy::start() {
    is_running_ = true;
    try {
        client_endpoint_ = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), client_port_);
        server_endpoint_ = udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), server_port_);

        while (true) {
            udp::endpoint remote_endpoint;
            boost::system::error_code error;
            std::size_t len = socket_.receive_from(boost::asio::buffer(recv_buffer_), remote_endpoint, udp::socket::message_end_of_record, error);
            if (!is_running_) break;
            if (error && error != boost::asio::error::message_size) {
                std::cerr << "Receive failed: " << error.message() << std::endl;
                continue;
            }
            int command = *(int*)&recv_buffer_[0];

            if (delay_ > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay_));

            std::uniform_real_distribution<float> distribution(0.0, 100.0);
            float random_number = distribution(generator);
            if (random_number < loss_percentage_ && command < 100) {
                continue;
            }

            if (test_1_skip_command__3) {
                if (test_skip_counter < test_skip_target_count) {
                    if (command == 3) {
                        std::cout << "[Proxy] Skip command: " << command << std::endl;
                        test_skip_counter++;
                        continue;
                    }
                } else {
                    test_1_skip_command__3 = false;
                }
            }

            if (test_2_skip_command__4) {
                if (test_skip_counter < test_skip_target_count) {
                    if (command == 4) {
                        std::cout << "[Proxy] Skip command: " << command << std::endl;
                        test_skip_counter++;
                        continue;
                    }
                } else {
                    test_2_skip_command__4 = false;
                }
            }

            if (test_3_skip_command__0) {
                if (test_skip_counter < test_skip_target_count) {
                    if (command == 0) {
                        std::cout << "[Proxy] Skip command: " << command << std::endl;
                        test_skip_counter++;
                        continue;
                    }
                } else {
                    test_3_skip_command__0 = false;
                }
            }

            if (remote_endpoint == client_endpoint_) {
                socket_.send_to(boost::asio::buffer(recv_buffer_, len), server_endpoint_, 0, error);
                if (error) {
                    std::cerr << "Send to server failed: " << error.message() << std::endl;
                }
            } else if (remote_endpoint == server_endpoint_) {
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

void UdpProxy::async_start() {
    if (is_running_) return;
    is_running_ = true;
    async_thread_ = std::thread([this] { start(); });
}

void UdpProxy::async_stop() {
    if (!is_running_) return;
    is_running_ = false;
    socket_.close();
    if (async_thread_.joinable()) {
        async_thread_.join();
    }
}

void UdpProxy::setDelay(int delay) {
    delay_ = delay;
}

void UdpProxy::setLossPercentage(float loss_percentage) {
    loss_percentage_ = loss_percentage;
}

void UdpProxy::runTest_1(int target_count) {
    test_1_skip_command__3 = true;
    test_skip_counter = 0;
    test_skip_target_count = target_count;
}

void UdpProxy::stopTest_1() {
    test_1_skip_command__3 = false;
    test_skip_counter = 0;
    test_skip_target_count = 0;
}

void UdpProxy::runTest_2(int target_count) {
    test_2_skip_command__4 = true;
    test_skip_counter = 0;
    test_skip_target_count = target_count;
}

void UdpProxy::stopTest_2() {
    test_2_skip_command__4 = false;
    test_skip_counter = 0;
    test_skip_target_count = 0;
}

void UdpProxy::runTest_3(int target_count) {
    test_3_skip_command__0 = true;
    test_skip_counter = 0;
    test_skip_target_count = target_count;
}

void UdpProxy::stopTest_3() {
    test_3_skip_command__0 = false;
    test_skip_counter = 0;
    test_skip_target_count = 0;
}

UdpProxy::~UdpProxy() {
    async_stop();
}
