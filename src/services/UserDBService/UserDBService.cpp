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

Worker UserDBService::findMostFreeWorker(int requiredCores) {
    Worker mostFreeWorker;
    int maxFreeCores = -1; // Start with -1 to ensure any valid worker with free cores will be considered.

    for (auto &worker: workers) {
        int freeCores = worker.cores - worker.usedCores;
        if (freeCores >= requiredCores && freeCores > maxFreeCores) {
            mostFreeWorker = worker;
            maxFreeCores = freeCores;
        }
    }

    return mostFreeWorker; // Will return default Worker if no suitable worker is found.
}


Client UserDBService::findClientByID(const std::string &id) {
    for (auto &client: clients) {
        if (client.id == id) {
            return client;
        }
    }
    throw std::runtime_error("Client not found");
}

Worker UserDBService::findWorkerByID(const std::string &id) {
    for (auto &worker: workers) {
        if (worker.id == id) {
            return worker;
        }
    }
    throw std::runtime_error("Worker not found");
}

void UserDBService::updateWorker(const Worker &worker) {
    for (auto &w: workers) {
        if (w.id == worker.id) {
            w = worker;
            return;
        }
    }
    throw std::runtime_error("Worker not found");
}

void UserDBService::printLog() {
    // print clients, workers and worker used cores
    std::cout << "Clients: " << std::endl;
    for (auto &client: clients) {
        std::cout << "Client: " << client.id << std::endl;
    }

    std::cout << "Workers: " << std::endl;
    for (auto &worker: workers) {
        std::cout << "Worker: " << worker.id << ", Cores: " << worker.cores << ", Used cores: " << worker.usedCores
                  << std::endl;
    }
}
