#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <thread>
#include <random>


using boost::asio::ip::udp;

constexpr int BUFFER_SIZE = 1024;

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
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_));

                // drop packet
                std::uniform_real_distribution<float> distribution(0.0, 100.0);

                float random_number = distribution(generator);
                if (random_number < loss_percentage_) {
                    continue;
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
//        socket_.close();
    }

    void setDelay(int delay) {
        delay_ = delay;
    }

    void setLossPercentage(float loss_percentage) {
        loss_percentage_ = loss_percentage;
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
};

int main() {
    UdpProxy udp_proxy(12223, 12224, 12226);
    udp_proxy.setDelay(500);
    udp_proxy.start();
    return 0;
}