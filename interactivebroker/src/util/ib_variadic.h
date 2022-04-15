//
// ib_variadic.h
// interactivebroker
//

#ifndef IB_VARIADIC_H
#define IB_VARIADIC_H

#include <iostream>
#include <sstream>

#include "ib_system.h"
#include "ib_configuration.h"
#include "ib_log.h"

namespace ib {
namespace util {

class LogFatal;
class LogError;
class LogWarn;
class LogInfo;
class LogDebug;
class LogTrace;

namespace macro {

inline std::ostringstream& operator<<(std::ostringstream &os,
    const std::ostringstream &s) {
  os << s.str();
  return os;
}

template<typename T>
void log(std::ostringstream &os, const T &t) {
  os << t;
}

template<typename T, typename ... Args>
void log(std::ostringstream &os, const T &t, const Args &... args) {
  os << t;
  macro::log<Args...>(os, args...);
}

template<typename T, typename ... Args>
void log(util::LoggerProxy<T> &logger, const char *file, int lineno,
    const char *function, const Args &... args) {
  std::ostringstream os;

  os << file << ":" << lineno << "(" << function << ") ";

  macro::log<Args...>(os, args...);

  logger << os.str() << util::endl;
}

#define LOGGER_TRACE(...) {                                                \
if (ib::util::Logger<ib::util::LogTrace>::isLevelOn())                     \
{                                                                          \
constexpr const char* basename(ib::util::system::basename(__FILE__));      \
ib::util::macro::log<ib::util::LogTrace>                                   \
(ib::util::trace, basename,                                                \
__LINE__, __FUNCTION__, __VA_ARGS__);                                      \
}                                                                          \
}

#define LOGGER_DEBUG(...) {                                                \
if (ib::util::Logger<ib::util::LogDebug>::isLevelOn())                     \
{                                                                          \
constexpr const char* basename(ib::util::system::basename(__FILE__));      \
ib::util::macro::log<ib::util::LogDebug>                                   \
(ib::util::debug, basename,                                                \
__LINE__, __FUNCTION__, __VA_ARGS__);                                      \
}                                                                          \
}

#define LOGGER_INFO(...) {                                                 \
if (ib::util::Logger<ib::util::LogInfo>::isLevelOn())                      \
{                                                                          \
constexpr const char* basename(ib::util::system::basename(__FILE__));      \
ib::util::macro::log<ib::util::LogInfo>                                    \
(ib::util::info, basename,                                                 \
__LINE__, __FUNCTION__, __VA_ARGS__);                                      \
}                                                                          \
}

#define LOGGER_WARN(...) {                                                 \
if (ib::util::Logger<ib::util::LogWarn>::isLevelOn())                      \
{                                                                          \
constexpr const char* basename(ib::util::system::basename(__FILE__));      \
ib::util::macro::log<ib::util::LogWarn>                                    \
(ib::util::warn, basename,                                                 \
__LINE__, __FUNCTION__, __VA_ARGS__);                                      \
}                                                                          \
}

#define LOGGER_ERROR(...) {                                                \
if (ib::util::Logger<ib::util::LogError>::isLevelOn())                     \
{                                                                          \
constexpr const char* basename(ib::util::system::basename(__FILE__));      \
ib::util::macro::log<ib::util::LogError>                                   \
(ib::util::error, basename,                                                \
__LINE__, __FUNCTION__, __VA_ARGS__);                                      \
}                                                                          \
}

#define LOGGER_FATAL(...) {                                                \
if (ib::util::Logger<ib::util::LogFatal>::isLevelOn())                     \
{                                                                          \
constexpr const char* basename(ib::util::system::basename(__FILE__));      \
ib::util::macro::log<ib::util::LogFatal>                                   \
(ib::util::fatal, basename,                                                \
__LINE__, __FUNCTION__, __VA_ARGS__);                                      \
}                                                                          \
}

} // ns macro
} // ns util
} // ns ib

#endif // IB_VARIADIC_H
