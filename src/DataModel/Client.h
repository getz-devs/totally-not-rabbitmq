//
// Created by Potato on 12.04.24.
//

#ifndef RABBIT_USER_H
#define RABBIT_USER_H

#include <string>
#include <nlohmann/json.hpp>
#include "protocol/Connection.h"

using json = nlohmann::json;

struct Client {
    std::string id;
    STIP::Connection *connection;
};

inline void to_json(json &j, const Client &c) {
    j = json{{"id", c.id}};
}

inline void from_json(const json &j, Client &c) {
    j.at("id").get_to(c.id);
}

#endif //RABBIT_USER_H
