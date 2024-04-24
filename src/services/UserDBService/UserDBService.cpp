//
// Created by Potato on 12.04.24.
//

#include "UserDBService.h"

void UserDBService::addClient(const Client &client) {
    clients.push_back(client);
};

void UserDBService::addWorker(const Worker &worker) {
    workers.push_back(worker);
};

void UserDBService::removeClient(const Client &client) {
    for (auto it = clients.begin(); it != clients.end(); it++) {
        if (it->id == client.id) {
            clients.erase(it);
            return;
        }
    }
}

void UserDBService::removeWorker(const Worker &worker) {
    for (auto it = workers.begin(); it != workers.end(); it++) {
        if (it->id == worker.id) {
            workers.erase(it);
            return;
        }
    }
}

Worker UserDBService::findMostFreeWorker() {
    Worker mostFreeWorker;
    mostFreeWorker.usedCores = INT_MAX;
    for (auto &worker: workers) {
        if (worker.usedCores < mostFreeWorker.usedCores) {
            mostFreeWorker = worker;
        }
    }
    return mostFreeWorker;
}