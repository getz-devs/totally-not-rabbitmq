//
// Created by Potato on 12.04.24.
//

#include "UserDBService.h"

Worker UserDBService::findMostFreeWorker() {
    Worker mostFreeWorker;
    mostFreeWorker.usedCores = INT_MAX;
    for (auto &worker : workers) {
        if (worker.usedCores < mostFreeWorker.usedCores) {
            mostFreeWorker = worker;
        }
    }
    return mostFreeWorker;
}