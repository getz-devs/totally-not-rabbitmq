//
// Created by Potato on 22.05.24.
//

#ifndef RABBIT_TASKQUEUE_H
#define RABBIT_TASKQUEUE_H

#include "protocol/STIP.h"
#include "server/STIPServer.h"
#include "protocol/Connection.h"
#include "DataModel/Message.h"
#include "Task.h"
#include <queue>

class TaskQueue {
public:
    void enqueue(const Task& task) {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(task);
    }

    // Try to dequeue a task that meets the core requirement.
    // Returns true and the task reference if successful,
    // otherwise false and the task reference remains unchanged.
    bool tryDequeue(Task& task, int requiredCores) {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (!taskQueue.empty() && taskQueue.front().cores <= requiredCores) {
            task = taskQueue.front();
            taskQueue.pop();
            return true;
        }
        return false;
    }

private:
    std::queue<Task> taskQueue;
    std::mutex queueMutex;
};

#endif //RABBIT_TASKQUEUE_H
