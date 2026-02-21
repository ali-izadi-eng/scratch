#pragma once
#include <string>
#include <vector>
#include <cstdint>

enum class LogLevel : uint8_t { Info, Warn, Error };

struct LogEntry {
  uint64_t tick{};
  LogLevel level{};
  std::string tag;
  std::string msg;
};

class Logger {
public:
  static void init();
  static void shutdown();

  static void info(std::string tag, std::string msg);
  static void warn(std::string tag, std::string msg);
  static void error(std::string tag, std::string msg);

  static const std::vector<LogEntry>& entries();
  static void clear();

private:
  static void push(LogLevel lvl, std::string tag, std::string msg);
};