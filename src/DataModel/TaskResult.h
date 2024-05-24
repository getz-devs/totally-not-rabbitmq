//
// Created by Potato on 11.04.24.
//

#ifndef RABBIT_TASKRESULT_H
#define RABBIT_TASKRESULT_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct TaskResult {
    std::string id;
    std::string data;
    int status;
};

inline void to_json(json &j, const struct TaskResult &tr) {
    j = json{{"id",     tr.id},
             {"data",   tr.data},
             {"status", tr.status}};
}

inline void from_json(const json &j, struct TaskResult &tr) {
    j.at("id").get_to(tr.id);
    j.at("data").get_to(tr.data);
    j.at("status").get_to(tr.status);
}

#endif //RABBIT_TASKRESULT_H
