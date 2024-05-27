#include "UserDBService.h"

#include <fstream>
#include <sstream>


void UserDBService::addClient(const Client &client) {
    clients.push_back(client);
    saveStateToFile("users.md");
};

void UserDBService::addWorker(const Worker &worker) {
    workers.push_back(worker);
    saveStateToFile("users.md");
};

void UserDBService::removeClient(const Client &client) {
    for (auto it = clients.begin(); it != clients.end(); it++) {
        if (it->id == client.id) {
            clients.erase(it);
            saveStateToFile("users.md");
            return;
        }
    }
}

void UserDBService::removeWorker(const Worker &worker) {
    for (auto it = workers.begin(); it != workers.end(); it++) {
        if (it->id == worker.id) {
            workers.erase(it);
            saveStateToFile("users.md");
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
            saveStateToFile("users.md");
            return client;
        }
    }
    throw std::runtime_error("Client not found");
}

Worker UserDBService::findWorkerByID(const std::string &id) {
    for (auto &worker: workers) {
        if (worker.id == id) {
            saveStateToFile("users.md");
            return worker;
        }
    }
    throw std::runtime_error("Worker not found");
}

void UserDBService::updateWorker(const Worker &worker) {
    for (auto &w: workers) {
        if (w.id == worker.id) {
            w = worker;
            saveStateToFile("users.md");

            return;
        }
    }
    throw std::runtime_error("Worker not found");
}

void UserDBService::modifyWorkerUsedCores(const std::string &id, int cores, bool increase) {
    for (auto &worker: workers) {
        if (worker.id == id) {
            std::cout << "Update cores: Worker before has used cores: " << worker.usedCores << std::endl;
            if (increase) {
                worker.usedCores += cores;
            } else {
                worker.usedCores -= cores;
            }
            std::cout << "Update cores: Worker " << worker.id << " used cores: " << worker.usedCores << std::endl;
            if (worker.usedCores < 0) {
                std::cerr << "[!] Worker " << worker.id << " used cores is negative. Resetting to 0." << std::endl;
                worker.usedCores = 0;
            }
            saveStateToFile("users.md");
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

void UserDBService::saveStateToFile(const std::string &filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    file << "| ID | Cores | Used Cores |\n";
    file << "|----|-------|------------|\n";
    for (const auto &worker: workers) {
        file << "| " << worker.id << " | " << worker.cores << " | " << worker.usedCores << " |\n";
    }

    file.close();
}
