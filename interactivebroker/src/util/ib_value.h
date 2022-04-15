//
// ib_value.h
// interactivebroker
//

#ifndef IB_VALUE_H
#define IB_VALUE_H

#include <algorithm>
#include <string>
#include <sstream>
#include <cassert>

#include "ib_const.h"

namespace ib {
namespace util {

class Value {
  
public:
  static inline std::string ltrim(const std::string &original) {
    std::string copy(original);

    copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](char c) {
      return !std::isspace(c);
    }));

    return copy;
  }

  static inline std::string rtrim(const std::string &original) {
    std::string copy(original);

    copy.erase(std::find_if(copy.rbegin(), copy.rend(), [](char c) {
      return !std::isspace(c);
    }).base(), copy.end());

    return copy;
  }

  static inline std::string trim(const std::string &s) {
    return Value::ltrim(Value::rtrim(s));
  }

  static inline std::string lowercase(const std::string &original) {
    std::string copy(original);

    std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);

    return copy;
  }

  static inline std::string newline() {
    std::ostringstream os;
    os << std::endl;
    return os.str();
  }

private:
  Value() = delete;

};

} // ns util
} // ns ib

#endif /* IB_VALUE_H */
