//
// ib_log.h
// interactivebroker
//

#ifndef IB_LOG_H
#define IB_LOG_H

#include <iostream>
#include <sstream>
#include <atomic>

namespace ib {
namespace util {

namespace time {
std::string getTimestamp();
} // ns time

namespace logger {
namespace id {
extern std::atomic<unsigned long long> counter;

const std::string& threadId();
} // ns id
} // ns logger

class LogFatal;
class LogError;
class LogWarn;
class LogInfo;
class LogDebug;
class LogTrace;

template<typename T>
class LoggerProxy;

class Logging {
  
public:
  static bool fatal() {
    return Logging::_fatal.load();
  }

protected:
  static std::atomic<bool> _fatal;

};

template<typename T>
class Logger: public Logging {
  
private:
  Logger() :
      _id(ib::util::logger::id::counter.fetch_add(1)) {
  }

  friend class LoggerProxy<T> ;

public:
  virtual ~Logger() {
    this->flush();
  }

  Logger(const Logger &logger) :
      _id(ib::util::logger::id::counter.fetch_add(1)) {
    this->_os << logger._os.str();
  }

  template<typename U>
  Logger<T>& operator<<(const U &u) {
    static bool isLevel(Logger<T>::isLevelOn());

    if (std::is_same<T, LogFatal>::value) {
      Logging::_fatal.store(true);
    }

    if (isLevel) {
      this->_os << u;
    }

    return (*this);
  }

  Logger<T>& operator<<(Logger<T>& (*fp)(Logger<T> &logger)) {
    return fp(*this);
  }

  void flush() {
    if (this->_os.str().length() > 0) {
      std::ostringstream os;

      os << time::getTimestamp() << " [" << ib::util::logger::id::threadId()
          << "|"
          // << _id
          // << "|"
          // << std::this_thread::get_id()
          // << "|"
          // << ((void*) this)
          << this->getPrefix() << "] " << this->_os.str();

      std::cout << os.str();

      this->clear();
    }
  }

  void clear() {
    this->_os.clear();
    this->_os.str("");
  }

  static bool isLevelOn();

  static bool isNoLevelOn();

  static std::string getTimestamp();

  const char* getPrefix() const;

private:
  Logger& operator=(const Logger&) = delete;

private:
  std::ostringstream _os;

  const long long _id;
};

template<typename T>
class LoggerProxy {
  
public:
  LoggerProxy() {
  }

  virtual ~LoggerProxy() {
    std::cout.flush();
  }

  template<typename U>
  Logger<T> operator<<(const U &data) {
    Logger<T> logger;

    logger << data;

    return logger;
  }

private:
  LoggerProxy(const LoggerProxy&) = delete;
  LoggerProxy& operator=(const LoggerProxy&) = delete;
};

extern LoggerProxy<LogFatal> fatal;
extern LoggerProxy<LogError> error;
extern LoggerProxy<LogWarn> warn;
extern LoggerProxy<LogInfo> info;
extern LoggerProxy<LogDebug> debug;
extern LoggerProxy<LogTrace> trace;

} // ns util
} // ns ib

#include "ib_value.h"

namespace ib {
namespace util {

template<typename T>
Logger<T>& endl(Logger<T> &logger) {
  static const std::string newline(ib::util::Value::newline());
  logger << newline;
  return logger;
}

} // ns util
} // ns ib

#include "ib_variadic.h" // leave this here!

#endif /* IB_LOG_H */

