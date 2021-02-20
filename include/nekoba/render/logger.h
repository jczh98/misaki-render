#pragma once

#include "fwd.h"
#include <fmt/format.h>

namespace nekoba {

class Formatter;

enum LogLevel : int {
  Trace = 0,    // Trace message, for extremely verbose debugging
  Debug = 100,  // Debug message, usually turned off
  Info = 200,   // More relevant debug / information message
  Warn = 300,   // Warning message
  Error = 400   // Error message, causes an exception to be thrown
};

class NEK_EXPORT Logger {
 public:
  Logger(LogLevel log_level = Debug);
  ~Logger();
  void log(LogLevel level, const char *filename, int line, const std::string &msg);
  void set_log_level(LogLevel level);
  void set_error_level(LogLevel level);
  LogLevel log_level() const { return m_log_level; }
  LogLevel error_level() const;
  void set_formatter(Formatter *formatter);
  Formatter *formatter();
  const Formatter *formatter() const;

 private:
  Formatter *m_formatter;
  LogLevel m_log_level, m_error_level;
};

NEK_EXPORT Logger *get_instanced_logger();

class Formatter {
  friend class Logger;

 public:
  Formatter();
  std::string format(LogLevel level,
                     const char *file, int line,
                     const std::string &msg);

  void set_has_date(bool value) { m_has_date = value; }
  bool has_date() { return m_has_date; }

  void set_has_log_level(bool value) { m_has_log_level = value; }
  bool has_log_level() { return m_has_log_level; }

 private:
  bool m_has_date;
  bool m_has_log_level;
};

namespace detail {

[[noreturn]] extern NEK_EXPORT void Throw(LogLevel level, const char *file, int line, const std::string &msg);

template <typename... Args>
NEK_INLINE static void Log(LogLevel level, const char *filename, int line, Args &&... args) {
  auto logger = get_instanced_logger();
  if (logger && level >= logger->log_level())
    logger->log(level, filename, line, fmt::format(std::forward<Args>(args)...));
}

}  // namespace detail

#define Log(level, ...)                                    \
  do {                                                     \
    nekoba::detail::Log(level, __FILE__, __LINE__, \
                                ##__VA_ARGS__);            \
  } while (0)

#define Throw(...)                                           \
  do {                                                       \
    nekoba::detail::Throw(Error, __FILE__, __LINE__, \
                                  fmt::format(__VA_ARGS__)); \
  } while (0)

#define NEK_NOT_IMPLEMENTED(Name) \
  Throw("{}::" Name "(): not implemented!", __FILE__)

}  // namespace nekoba