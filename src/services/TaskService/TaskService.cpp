//
// Created by Serge on 26.03.2024.
//

#include "TaskService.h"


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
}

void TaskService::updateTask(const Task& task) {
    for (auto &t: tasks) {
        if (t.id == task.id) {
            t = task;
            return;
        }
    }
    throw std::runtime_error("Task not found");
}

void TaskService::changeTaskStatus(const std::string &id, TaskStatus status) {
    for (auto &task: tasks) {
        if (task.id == id) {
            task.status = status;
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
