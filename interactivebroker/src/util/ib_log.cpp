//
//  ib_log.cpp
//  interactivebroker
//

#include <sstream>

#include "ib_configuration.h"
#include "ib_log.h"
#include "ib_time.h"

namespace ib {
namespace util {

LoggerProxy<LogFatal> fatal;
LoggerProxy<LogError> error;
LoggerProxy<LogWarn> warn;
LoggerProxy<LogInfo> info;
LoggerProxy<LogDebug> debug;
LoggerProxy<LogTrace> trace;

namespace logger {
namespace id {
std::atomic<unsigned long long> counter(0);

const std::string& threadId() {
  static const std::string prefix("thread-");

  static std::atomic<int> count { 1 };

  const thread_local std::string name(
      prefix + std::to_string(count.fetch_add(1)));

  return name;
}
} // ns id
} // ns logger

std::atomic<bool> Logging::_fatal;

template<typename T>
bool Logger<T>::isNoLevelOn() {
  static const bool isLevel(
      static_cast<int>(Configuration::configuration()->isLevelTrace())
          + // NOTE: '+' intentional - instead of '&'
          static_cast<int>(Configuration::configuration()->isLevelDebug())
          + static_cast<int>(Configuration::configuration()->isLevelInfo())
          + static_cast<int>(Configuration::configuration()->isLevelWarn())
          + static_cast<int>(Configuration::configuration()->isLevelError())
          + static_cast<int>(Configuration::configuration()->isLevelFatal())
          == 0);

  return isLevel;
}

namespace time {
std::string getTimestamp() {
  return Time::timestamp();
}
}

//

template<>
const char* Logger<LogFatal>::getPrefix() const {
  return ib::util::Const::_CONST_ENVVAR_LOGLEVEL_FATAL;
}

template<>
const char* Logger<LogError>::getPrefix() const {
  return ib::util::Const::_CONST_ENVVAR_LOGLEVEL_ERROR;
}

template<>
const char* Logger<LogWarn>::getPrefix() const {
  return ib::util::Const::_CONST_ENVVAR_LOGLEVEL_WARN;
}

template<>
const char* Logger<LogInfo>::getPrefix() const {
  return ib::util::Const::_CONST_ENVVAR_LOGLEVEL_INFO;
}

template<>
const char* Logger<LogDebug>::getPrefix() const {
  return ib::util::Const::_CONST_ENVVAR_LOGLEVEL_DEBUG;
}

template<>
const char* Logger<LogTrace>::getPrefix() const {
  return ib::util::Const::_CONST_ENVVAR_LOGLEVEL_TRACE;
}

//

template<>
bool Logger<LogTrace>::isLevelOn() {
  static const bool isLevel(Configuration::configuration()->isLevelTrace());

  return isLevel;
}

template<>
bool Logger<LogDebug>::isLevelOn() {
  static const bool isLevel(
      Configuration::configuration()->isLevelDebug()
          || ib::util::Logger < ib::util::LogTrace > ::isLevelOn());

  return isLevel;
}

template<>
bool Logger<LogInfo>::isLevelOn() {
  static const bool isLevel(
      isNoLevelOn() ?
          true :
          (Configuration::configuration()->isLevelInfo()
              || ib::util::Logger < ib::util::LogDebug > ::isLevelOn()));

  return isLevel;
}

template<>
bool Logger<LogWarn>::isLevelOn() {
  static const bool isLevel(
      isNoLevelOn() ?
          true :
          (Configuration::configuration()->isLevelWarn()
              || ib::util::Logger < ib::util::LogInfo > ::isLevelOn()));

  return isLevel;
}

template<>
bool Logger<LogError>::isLevelOn() {
  static const bool isLevel(
      isNoLevelOn() ?
          true :
          (Configuration::configuration()->isLevelError()
              || ib::util::Logger < ib::util::LogWarn > ::isLevelOn()));

  return isLevel;
}

template<>
bool Logger<LogFatal>::isLevelOn() {
  static const bool isLevel(
      isNoLevelOn() ?
          true :
          (Configuration::configuration()->isLevelFatal()
              || ib::util::Logger < ib::util::LogError > ::isLevelOn()));

  return isLevel;
}

} // ns util
} // ns ib

