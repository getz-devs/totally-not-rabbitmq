#ifndef RABBIT_WORKER_H
#define RABBIT_WORKER_H

#include <string>
#include <nlohmann/json.hpp>
#include "protocol/Connection.h"

using json = nlohmann::json;

struct Worker {
    std::string id;
    int cores;
    int usedCores;
    STIP::Connection *connection;
};

inline void to_json(json &j, const Worker &w) {
    j = json{{"id",        w.id},
             {"cores",     w.cores},
             {"usedCores", w.usedCores}};
}

inline void from_json(const json &j, Worker &w) {
    j.at("id").get_to(w.id);
    j.at("cores").get_to(w.cores);
    j.at("usedCores").get_to(w.usedCores);
}

#endif //RABBIT_WORKER_H
