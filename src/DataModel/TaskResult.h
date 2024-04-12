//
// Created by Potato on 11.04.24.
//

#ifndef RABBIT_TASKRESULT_H
#define RABBIT_TASKRESULT_H

#include <string>
#include <nlohmann/json.hpp>

// TODO: обернуть в Message

using json = nlohmann::json;

struct TaskResult {
    int id;
    std::string data;
    int status;
};

void to_json(json &j, const TaskResult &tr) {
    j = json{{"id",     tr.id},
             {"data",   tr.data},
             {"status", tr.status}};
}

void from_json(const json &j, TaskResult &tr) {
    j.at("id").get_to(tr.id);
    j.at("data").get_to(tr.data);
    j.at("status").get_to(tr.status);
}

#endif //RABBIT_TASKRESULT_H
