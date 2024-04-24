//
// Created by Serge on 26.03.2024.
//

#ifndef RABBIT_TASKS_H
#define RABBIT_TASKS_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// TODO: вероятно он не нужен (оболочка в json)

struct Task {
    int id;
    std::string func;
    std::string input;
    std::string output;
    int cores;
    int status;
    std::string worker_hash_id;
    std::string client_hash_id;
};

void to_json(json &j, const Task &t) {
    j = json{{"id",             t.id},
             {"func",           t.func},
             {"input",          t.input},
             {"output",         t.output},
             {"cores",          t.cores},
             {"status",         t.status},
             {"worker_hash_id", t.worker_hash_id},
             {"client_hash_id", t.client_hash_id}};
}

void from_json(const json &j, Task &t) {
    j.at("id").get_to(t.id);
    j.at("func").get_to(t.func);
    j.at("input").get_to(t.input);
    j.at("output").get_to(t.output);
    j.at("cores").get_to(t.cores);
    j.at("status").get_to(t.status);
    j.at("worker_hash_id").get_to(t.worker_hash_id);
    j.at("client_hash_id").get_to(t.client_hash_id);
}

#endif //RABBIT_TASKS_H
