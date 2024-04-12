//
// Created by Potato on 12.04.24.
//

#ifndef RABBIT_USER_H
#define RABBIT_USER_H

#include <string>
#include "protocol/Connection.h"

struct Worker {
    std::string id;
    int cores;
    int usedCores;
    STIP::Connection *connection;
};

#endif //RABBIT_USER_H
