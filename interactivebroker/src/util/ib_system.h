//
// ib_system.h
// interactivebroker
//

#ifndef IB_SYSTEM_H
#define IB_SYSTEM_H

#include <string>
#include <cstring>
#include <atomic>

namespace ib {
namespace util {
namespace system {

#ifdef __APPLE__
  #include <TargetConditionals.h>
  #if TARGET_OS_IPHONE && TARGET_OS_SIMULATOR
    static_assert(0, "platform NOT suppprted :)");
  #elif TARGET_OS_IPHONE && TARGET_OS_MACCATALYST
    static_assert(0, "platform NOT suppprted :)");
  #elif TARGET_OS_IPHONE
    static_assert(0, "platform NOT suppprted :)");
  #else
    constexpr char path_separator = '/';
  #endif
#elif __linux
  constexpr char path_separator = '/';
#else
static_assert(0, "platform NOT suppprted :)");
#endif

constexpr const char* str_end(const char *s) {
  return *s ? str_end(s + 1) : s;
}

constexpr bool str_slant(const char *s) {
  return *s == path_separator ? true : (*s ? str_slant(s + 1) : false);
}

constexpr const char* r_slant(const char *s) {
  return *s == path_separator ? (s + 1) : r_slant(s - 1);
}

constexpr const char* basename(const char *s) {
  return str_slant(s) ? r_slant(str_end(s)) : s;
}

//

inline std::string getenv(const std::string &name) {
  char *value { std::getenv(name.c_str()) };

  if (value != nullptr && strlen(value) > 0) {
    return std::string(value);
  }

  return std::string();
}

namespace threading {
extern std::atomic<bool> _shutdown;
} // ns threading

} // ns system
} // ns util
} // ns ib

#endif /* IB_SYSTEM_H */
