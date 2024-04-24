//
// Created by Serge on 23.03.2024.
//

#ifndef RABBIT_RABBITSERVER_H
#define RABBIT_RABBITSERVER_H

#include <boost/asio.hpp>
#include <sqlite_orm/sqlite_orm.h>
#include "protocol/Connection.h"
#include "services/TaskService/TaskService.h"
#include "services/UserDBService/UserDBService.h"
#include <nlohmann/json.hpp>

using boost::asio::ip::udp;
using namespace sqlite_orm;
using json = nlohmann::json;

class RabbitServer;

typedef void (RabbitServer::*func_type)(int, json, int);

typedef std::map<std::string, func_type> func_map_type;

class RabbitServer {
public:
    RabbitServer(int port);

    void init();

    void startPolling();

    void processConnection(STIP::Connection *connection);

    ~RabbitServer() {
        delete server_socket;
    }

private:
    int port;
    boost::asio::io_context io_context;
    udp::socket *server_socket;

    TaskService taskService;
    UserDBService userDBService;

    // json parseMessage(std::string message);
    static bool validateRequest(json request);

    func_map_type handlers;
};


#endif //RABBIT_RABBITSERVER_H
