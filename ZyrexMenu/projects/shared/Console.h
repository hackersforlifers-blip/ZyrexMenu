#pragma once
#include <string>
#include <vector>
#include <chrono>

enum class LogLevel {
    Debug,
    Info,
    Warn,
    Error
};

struct LogEntry {
    uint64_t id;
    LogLevel level;
    std::string text;
    std::chrono::system_clock::time_point timestamp;
};

namespace Console {
    void Initialize();
    void Debug(const char* fmt, ...);
    void Info(const char* fmt, ...);
    void Warn(const char* fmt, ...);
    void Error(const char* fmt, ...);
    void Clear();
    std::vector<LogEntry> GetLogs();
    void SetTitle(const char* title);
}
