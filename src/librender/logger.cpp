#include <nekoba/render/logger.h>
#include <iostream>

namespace nekoba {

Logger *get_instanced_logger() {
  static std::unique_ptr<Logger> logger;
  static std::once_flag flag;
  std::call_once(flag, [&]() {
    logger = std::make_unique<Logger>(Info);
    auto formatter = new Formatter();
    logger->set_formatter(formatter);
#if defined(NDEBUG)
    logger->set_log_level(Info);
#else
    logger->set_log_level(Debug);
#endif
  });
  return logger.get();
}

// -----------------------Logger-------------------------
Logger::Logger(LogLevel log_level) : m_log_level(log_level), m_error_level(Error) {}
Logger::~Logger() {
  delete m_formatter;
}
void Logger::set_log_level(LogLevel level) {
  m_log_level = level;
}
void Logger::set_error_level(LogLevel level) {
  m_error_level = level;
}
LogLevel Logger::error_level() const {
  return m_error_level;
}
void Logger::set_formatter(Formatter *formatter) {
  m_formatter = formatter;
}
Formatter *Logger::formatter() {
  return m_formatter;
}
const Formatter *Logger::formatter() const {
  return m_formatter;
}
#undef Throw
void Logger::log(LogLevel level, const char *filename, int line, const std::string &msg) {
  if (level < m_log_level)
    return;
  else if (level >= m_error_level)
    detail::Throw(level, filename, line, msg);

  if (!m_formatter) {
    std::cerr << "PANIC: Logging has not been properly initialized!" << std::endl;
    abort();
  }

  std::string text = m_formatter->format(level, filename, line, msg);
  fmt::print(text);
}
// --------------------------Formatter-------------------
Formatter::Formatter() : m_has_date(true), m_has_log_level(true) {}
std::string Formatter::format(LogLevel level, const char *file, int line, const std::string &msg) {
  std::ostringstream oss;
  std::istringstream iss(msg);
  char buffer[128];
  std::string msg_line;
  time_t time_ = std::time(nullptr);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S ", std::localtime(&time_));
  int line_idx = 0;

  while (std::getline(iss, msg_line) || line_idx == 0) {
    if (line_idx > 0)
      oss << '\n';

    if (m_has_date)
      oss << buffer;

    if (m_has_log_level) {
      switch (level) {
        case Trace: oss << "TRACE "; break;
        case Debug: oss << "DEBUG "; break;
        case Info:  oss << "INFO  "; break;
        case Warn:  oss << "WARN  "; break;
        case Error: oss << "ERROR "; break;
        default:     oss << "CUSTM "; break;
      }
    }

    if (line != -1 && file)
        oss << "[" << fs::path(file).filename() << ":" << line << "] ";

    oss << msg_line;
    line_idx++;
  }
  oss << '\n';
  return oss.str();
}

namespace detail {

void Throw(LogLevel level, const char *file, int line, const std::string &msg) {
  Formatter formatter;
  formatter.set_has_date(false);
  formatter.set_has_log_level(false);
  throw std::runtime_error(formatter.format(level, file, line, msg));
}

}


}