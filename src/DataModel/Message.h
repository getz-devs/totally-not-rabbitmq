//
// Created by Potato on 12.04.24.
//

#ifndef RABBIT_MESSAGE_H
#define RABBIT_MESSAGE_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// TODO: обернуть все нижестоящие объекты в этот высооуровневый класс

enum class MessageType : int {
    Register = 0, TaskRequest = 1, TaskResult = 2
};

// example enum type declaration
enum TaskState {
    Register,
    TaskRequest,
    TaskResult,
    Invalid = -1
};

// map TaskState values to JSON as strings
NLOHMANN_JSON_SERIALIZE_ENUM(TaskState, {
    { Register, "register" },
    { TaskRequest, "request" },
    { TaskResult, "result" },
    { Invalid, nullptr },
})

struct Message {
    MessageType action;
    std::string data;
};

void to_json(nlohmann::json &j, const Message &m) {
    j = json{{"action", m.action},
             {"data",   m.data}};
}

void from_json(const nlohmann::json &j, Message &m) {
    j.at("action").get_to(m.action);
    j.at("data").get_to(m.data);
}

#endif //RABBIT_MESSAGE_H
