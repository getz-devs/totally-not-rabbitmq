//
// Created by Serge on 11.04.2024.
//

#include "SessionKiller.h"

namespace STIP {

    void SessionKiller::registerSessionTimeout(uint32_t sessionId, uint32_t timeout, std::function<void()> callback) {
            std::lock_guard<std::mutex> lock(mtx);
            timers[sessionId].setTimeout(timeout);
            timers[sessionId].setCallback(callback);
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