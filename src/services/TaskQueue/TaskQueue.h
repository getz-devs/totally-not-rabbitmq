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
#include <mutex>
#include <fstream>
#include <sstream>
#include <stdexcept>

class TaskQueue {
public:
    void enqueue(const Task &task) {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(task);
        saveStateAsMarkdown("tasksQueue.md");
    }

    // Try to dequeue a task that meets the core requirement.
    // Returns true and the task reference if successful,
    // otherwise false and the task reference remains unchanged.
    bool tryDequeue(Task &task, int requiredCores) {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (!taskQueue.empty() && taskQueue.front().cores <= requiredCores) {
            task = taskQueue.front();
            taskQueue.pop();
            saveStateAsMarkdown("tasksQueue.md");
            return true;
        }
        return false;
    }

    void saveStateAsMarkdown(const std::string &filename) {
//        std::lock_guard<std::mutex> lock(queueMutex);
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file");
        }

        // Write the markdown table header
        file << "| Task ID | Cores Required | Description |\n";
        file << "|---------|----------------|-------------|\n";

        // Write each task as a row in the markdown table
        std::queue<Task> tempQueue = taskQueue; // Copy the queue for iteration
        while (!tempQueue.empty()) {
            Task task = tempQueue.front();
            tempQueue.pop();

            // Assuming Task has getId(), getCores(), and getDescription() methods
            file << "| " << task.id << " | " << task.cores << " | " << task.func << " |\n";
        }

        file.close();
    }

private:
    std::queue<Task> taskQueue;
    std::mutex queueMutex;
};

#endif //RABBIT_TASKQUEUE_H
