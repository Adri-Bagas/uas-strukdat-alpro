#pragma once
#include <string>
#include "../utils/DoubleLinkedList.hpp"

class LogManager {
public:
    struct LogEntry {
        std::string timestamp;
        std::string message;
    };

private:
    Utils::DoubleLinkedList<LogEntry> logs;
    int max_logs;

public:
    LogManager(int max_capacity = 100);

    void add_log(const std::string& timestamp, const std::string& message);
    void clear_logs();

    Utils::DoubleLinkedList<LogEntry>& get_logs() { return logs; }
    const Utils::DoubleLinkedList<LogEntry>& get_logs() const { return logs; }
};
