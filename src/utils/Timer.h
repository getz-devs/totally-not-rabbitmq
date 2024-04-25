//
// Created by Serge on 08.03.2024.
//

#ifndef RABBIT_TIMER_H
#define RABBIT_TIMER_H

#include <chrono>
#include <thread>
#include <functional>
#include <atomic>




//        Есть поток фьючер который по таймауту вызовет колбэк
//        Если же условие выполнится раньше - перезапускаем
//        А условие раньше - это thread lock. Который разблокируется после вызова функции reset
class Timer {
private:
    std::chrono::milliseconds timeout;
    std::thread timerThread;
    std::atomic<bool> running = false;
    std::function<void()> callback;

    std::condition_variable cv;
    std::mutex cvMtx;

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
        if (running) {
            return;
        }
        running = true;
        timerThread = std::thread([this] {
            while (running) {
                std::unique_lock<std::mutex> lock(cvMtx);
                if (cv.wait_for(lock, timeout) == std::cv_status::timeout) {
                    callback();
                }
            }
        });

    }

    bool isRunning() {
        return running;
    }

    void stop() {
        if (!running) {
            return;
        }
        running = false;
        cv.notify_one();
        timerThread.join();
    }

    void reset() {
        cv.notify_one();
    }



    ~Timer() {
        stop();
    }
};


#endif //RABBIT_TIMER_H
