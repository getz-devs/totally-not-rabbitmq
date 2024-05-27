#include <iostream>
#include <boost/asio.hpp>


#include "client/STIPClient.h"

using namespace STIP;

#include <csignal>

int main() {
    try {
        boost::asio::io_context io_context;

        udp::resolver resolver(io_context);
        udp::endpoint server_endpoint = *resolver.resolve(udp::v4(), "localhost", "12345").begin();

        udp::socket socket(io_context);
        socket.open(udp::v4());

        STIPClient client(socket);
        client.startListen();

        Connection *connection = client.connect(server_endpoint, 1000, 3);
        std::cout << "Connection accepted\n\n" << std::endl;

        std::string message = "Hello, I'm Ilya";
        connection->sendMessage(message);

        ReceiveMessageSession *received = connection->receiveMessage();
        std::cout << "Received message: " << received->getDataAsString() << std::endl;

    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
