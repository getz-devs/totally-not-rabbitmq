//
// Created by Potato on 12.04.24.
//

#ifndef RABBIT_USERDBSERVICE_H
#define RABBIT_USERDBSERVICE_H

#include "DataModel/Client.h"
#include "DataModel/Worker.h"

class UserDBService {
public:
    UserDBService() {
        clients = std::vector<Client>();
        workers = std::vector<Worker>();
    };

    void addClient(const Client& client);
    void addWorker(const Worker& worker);

    void removeClient(const Client& client);
    void removeWorker(const Worker& worker);

    void updateWorker(const Worker& worker);

    Worker fingByID(const std::string& id);
    Worker findMostFreeWorker();


private:
    std::vector<Client> clients;
    std::vector<Worker> workers;
};

#endif //RABBIT_USERDBSERVICE_H
