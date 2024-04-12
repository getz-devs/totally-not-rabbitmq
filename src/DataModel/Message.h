//
// Created by Potato on 12.04.24.
//

#ifndef RABBIT_MESSAGE_H
#define RABBIT_MESSAGE_H

#include <string>

// TODO: обернуть все нижестоящие объекты в этот высооуровневый класс

struct Message {
    int action; // 0 - auth, 1 -
    std::string data;
};

#endif //RABBIT_MESSAGE_H
