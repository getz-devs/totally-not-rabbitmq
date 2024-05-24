//
// Created by Serge on 26.03.2024.
//

#include "TaskService.h"
#include <fstream>
#include <sstream>


TaskService::TaskService() {
    tasks = std::vector<Task>();
    // storage.sync_schema(); -- remove DB for now
}

void TaskService::addTask(Task task) {
    if (task.id.empty()) {
        task.id = newTaskID();
    }
    task.status = TaskStatus::Queued;
    tasks.push_back(task);
#ifdef SERVER_LOG_FILES
    saveTasksToFile("tasks.md");
#endif

}

void TaskService::updateTask(const Task &task) {
    for (auto &t: tasks) {
        if (t.id == task.id) {
            t = task;
#ifdef SERVER_LOG_FILES
            saveTasksToFile("tasks.md");
#endif
            return;
        }
    }
    throw std::runtime_error("Task not found");
}

void TaskService::changeTaskStatus(const std::string &id, TaskStatus status) {
    for (auto &task: tasks) {
        if (task.id == id) {
            task.status = status;
#ifdef SERVER_LOG_FILES
            saveTasksToFile("tasks.md");
#endif
            return;
        }
    }
    throw std::runtime_error("Task not found");
}

std::string TaskService::newTaskID() {
    return "task-" + std::to_string(tasks.size());
}

Task TaskService::findTaskByID(std::string id) {
    for (auto &task: tasks) {
        if (task.id == id) {
            return task;
        }
    }
    throw std::runtime_error("Task not found");
}

void TaskService::saveTasksToFile(const std::string &filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    // Write the header
    file << "| Task ID | Status | Description | Worker |\n";
    file << "|---------|--------|-------------|--------|\n";

    // Write each task as a row
    for (const auto &task: tasks) {
        file << "| " << task.id << " | " << static_cast<int>(task.status) << " | " << task.func << " | " << task.worker_hash_id << " |\n";
    }

    file.close();
}
