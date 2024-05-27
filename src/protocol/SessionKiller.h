#ifndef RABBIT_SESSIONKILLER_H
#define RABBIT_SESSIONKILLER_H

#include <iostream>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "utils/Timer.h"

namespace STIP {

    class SessionKiller {
    private:
        std::map<uint32_t, Timer> timers;
        std::mutex mtx;
    public:
        SessionKiller() = default;
        ~SessionKiller() = default;
        void registerSessionTimeout(uint32_t sessionId, int timeout, std::function<void()> callback);
        void deleteSessionTimeout(uint32_t sessionId);
        void resetSessionTimeout(uint32_t sessionId);
    };

} // STIP

#endif //RABBIT_SESSIONKILLER_H
