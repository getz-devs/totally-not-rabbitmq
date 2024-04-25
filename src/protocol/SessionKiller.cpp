//
// Created by Serge on 11.04.2024.
//

#include "SessionKiller.h"

#include <utility>

namespace STIP {

    void SessionKiller::registerSessionTimeout(uint32_t sessionId, int timeout, std::function<void()> callback) {
            std::lock_guard<std::mutex> lock(mtx);
            timers[sessionId].setTimeout(timeout);
            timers[sessionId].setCallback(std::move(callback));
            timers[sessionId].start();
        }

        void SessionKiller::deleteSessionTimeout(uint32_t sessionId) {
            std::lock_guard<std::mutex> lock(mtx);
            timers.erase(sessionId);
        }

        void SessionKiller::resetSessionTimeout(uint32_t sessionId) {
            std::lock_guard<std::mutex> lock(mtx);
            timers[sessionId].reset();
        }
} // STIP