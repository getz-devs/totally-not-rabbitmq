//
// Created by Potato on 12.04.24.
//

#ifndef RABBIT_USER_H
#define RABBIT_USER_H

#include <string>
#include "protocol/Connection.h"

struct Client {
    std::string id;
    STIP::Connection *connection;
};

#endif //RABBIT_USER_H
