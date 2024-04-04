//
// Created by Serge on 26.03.2024.
//

#ifndef RABBIT_TASKS_H
#define RABBIT_TASKS_H

#include <string>

struct Task {
    int id;
    std::string queue;
    std::string message;
    std::string result_message;
    int internal_status;
    std::string worker_hash_id;
    std::string client_hash_id;
};


#endif //RABBIT_TASKS_H
