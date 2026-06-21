#include "LogManager.hpp"

LogManager::LogManager(int max_capacity) : max_logs(max_capacity) {}

void LogManager::add_log(const std::string& timestamp, const std::string& message) {
    LogEntry entry;
    entry.timestamp = timestamp;
    entry.message = message;
    
    logs.push_back(entry);
    
    while (logs.size() > max_logs) {
        logs.pop_front();
    }
}

void LogManager::clear_logs() {
    logs.clear();
}
