#include "Console.h"
#include <cstdio>
#include <cstdarg>
#include <mutex>
#include <atomic>
#include <algorithm>

constexpr size_t kMaxLogEntries = 5000;
constexpr size_t kMaxMsgLength = 2048;

static std::mutex s_mutex;
static std::vector<LogEntry> s_logs;
static std::atomic<uint64_t> s_next_id{ 1 };

void Console::Initialize()
{
}

static void AppendLog(LogLevel level, const char* fmt, va_list args)
{
    char buf[kMaxMsgLength];
    vsnprintf(buf, kMaxMsgLength, fmt, args);
    buf[kMaxMsgLength - 1] = '\0';

    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_logs.size() >= kMaxLogEntries)
        s_logs.erase(s_logs.begin());
    s_logs.push_back({
        s_next_id++,
        level,
        buf,
        std::chrono::system_clock::now()
    });
}

void Console::Debug(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    AppendLog(LogLevel::Debug, fmt, args);
    va_end(args);
}

void Console::Info(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    AppendLog(LogLevel::Info, fmt, args);
    va_end(args);
}

void Console::Warn(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    AppendLog(LogLevel::Warn, fmt, args);
    va_end(args);
}

void Console::Error(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    AppendLog(LogLevel::Error, fmt, args);
    va_end(args);
}

void Console::Clear()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_logs.clear();
}

std::vector<LogEntry> Console::GetLogs()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    return s_logs;
}

void Console::SetTitle(const char*)
{
}
