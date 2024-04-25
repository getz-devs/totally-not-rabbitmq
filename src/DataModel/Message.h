//
// Created by Potato on 12.04.24.
//

#ifndef RABBIT_MESSAGE_H
#define RABBIT_MESSAGE_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// TODO: обернуть все нижестоящие объекты в этот высооуровневый класс

// example enum type declaration
enum MessageType {
    RegisterClient,
    RegisterWorker,
    TaskRequest,
    TaskResult,
    Invalid = -1
};

// map TaskState values to JSON as strings
NLOHMANN_JSON_SERIALIZE_ENUM(MessageType, {
    { RegisterClient, "registerClient" },
    { RegisterWorker, "registerWorker" },
    { TaskRequest, "request" },
    { TaskResult, "result" },
    { Invalid, nullptr },
})

struct Message {
    MessageType action;
    std::string data;
};

inline void to_json(nlohmann::json &j, const Message &m) {
    j = json{{"action", m.action},
             {"data",   m.data}};
}

inline void from_json(const nlohmann::json &j, Message &m) {
    j.at("action").get_to(m.action);
    j.at("data").get_to(m.data);
}

#endif //RABBIT_MESSAGE_H
