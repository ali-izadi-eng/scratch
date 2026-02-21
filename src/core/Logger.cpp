#include "Logger.h"
#include "Time.h"
#include <mutex>

static std::vector<LogEntry> g_entries;
static std::mutex g_mtx;

void Logger::init() {
  std::scoped_lock lk(g_mtx);
  g_entries.clear();
}

void Logger::shutdown() {
  // اگر خواستی بعداً خروجی فایل هم اضافه می‌کنیم
}

void Logger::push(LogLevel lvl, std::string tag, std::string msg) {
  std::scoped_lock lk(g_mtx);
  g_entries.push_back(LogEntry{
    .tick = Time::tickCount(),
    .level = lvl,
    .tag = std::move(tag),
    .msg = std::move(msg)
  });

  if (g_entries.size() > 5000) g_entries.erase(g_entries.begin(), g_entries.begin() + 1000);
}

void Logger::info(std::string tag, std::string msg)  { push(LogLevel::Info,  std::move(tag), std::move(msg)); }
void Logger::warn(std::string tag, std::string msg)  { push(LogLevel::Warn,  std::move(tag), std::move(msg)); }
void Logger::error(std::string tag, std::string msg) { push(LogLevel::Error, std::move(tag), std::move(msg)); }

const std::vector<LogEntry>& Logger::entries() { return g_entries; }

void Logger::clear() {
  std::scoped_lock lk(g_mtx);
  g_entries.clear();
}