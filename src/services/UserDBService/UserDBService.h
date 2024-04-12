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

    void addClient(const Client& client) { clients.push_back(client); };
    void addWorker(const Worker& worker) { workers.push_back(worker); };

    Worker findMostFreeWorker();


private:
    std::vector<Client> clients;
    std::vector<Worker> workers;
};

#endif //RABBIT_USERDBSERVICE_H
