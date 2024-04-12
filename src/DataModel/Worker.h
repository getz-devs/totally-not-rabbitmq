//
// Created by Potato on 12.04.24.
//

#ifndef RABBIT_WORKER_H
#define RABBIT_WORKER_H

#include <string>
#include "protocol/Connection.h"

struct Worker {
    std::string id;
    int cores;
    int usedCores;
    STIP::Connection *connection;
};

#endif //RABBIT_WORKER_H
