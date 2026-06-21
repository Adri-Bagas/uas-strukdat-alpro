#include "Logger.hpp"
#include <fstream>
#include <iostream>

namespace Utils {
namespace Logger {
    const char* LOG_FILE = "/tmp/game_debug.log";

    void log(const std::string& message) {
        std::ofstream file(LOG_FILE, std::ios::app);
        if (file.is_open()) {
            file << message << std::endl;
        }
    }

    void clear() {
        std::ofstream file(LOG_FILE, std::ios::trunc);
    }
}
}