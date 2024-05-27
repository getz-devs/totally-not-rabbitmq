#ifndef RABBIT_UDPPROXY_H
#define RABBIT_UDPPROXY_H


#ifndef UDPPROXY_HPP
#define UDPPROXY_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <thread>
#include <random>
#include <atomic>
#include "protocol/STIP.h"

constexpr int BUFFER_SIZE = STIP::MAX_UDP_SIZE;

class UdpProxy {
public:
    UdpProxy(int client_port, int server_port, int proxy_port);
    void start();
    void async_start();
    void async_stop();
    void setDelay(int delay);
    void setLossPercentage(float loss_percentage);
    void runTest_1(int target_count);
    void stopTest_1();
    void runTest_2(int target_count);
    void stopTest_2();
    void runTest_3(int target_count);
    void stopTest_3();
    ~UdpProxy();

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint client_endpoint_;
    boost::asio::ip::udp::endpoint server_endpoint_;
    boost::array<char, BUFFER_SIZE> recv_buffer_;
    std::thread async_thread_;
    bool is_running_ = false;
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
    bool test_3_skip_command__0 = false;
};

#endif // UDPPROXY_HPP


#endif //RABBIT_UDPPROXY_H
