//
// Created by Serge on 08.03.2024.
//

#ifndef RABBIT_TIMER_H
#define RABBIT_TIMER_H

#include <chrono>
#include <thread>
#include <functional>
#include <atomic>


class Timer {
private:
    std::chrono::milliseconds timeout;
    std::thread timerThread;
    std::atomic<bool> running = false;
    std::function<void()> callback;

public:
    Timer() {
        // 60 sec
        timeout = std::chrono::milliseconds(60000);
    }

    void setCallback(std::function<void()> callback) {
        this->callback = callback;
    }

    void setTimeout(int milliseconds) {
        timeout = std::chrono::milliseconds(milliseconds);
    }

    void start() {
        running = true;
        timerThread = std::thread([this] {
                while (running) {
                    std::this_thread::sleep_for(timeout);
                    if (running && callback) {
                        this->callback();
                    }
                }
        });
        timerThread.detach();
    }

    bool isRunning() {
        return running;
    }

    void stop() {
        running = false;
    }



    ~Timer() {
        stop();
    }
};


#endif //RABBIT_TIMER_H
