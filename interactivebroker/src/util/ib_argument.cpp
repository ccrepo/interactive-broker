//
//  ib_argument.cpp
//  interactivebroker
//

#include <cstring>

#include "ib_argument.h"

namespace ib {
namespace util {

std::unique_ptr<Argument> Argument::_argument(
    new Argument(false, 0, std::vector<std::string>()));

bool Argument::init(int argc, const char *argv[]) {
  
  static std::atomic<bool> guard(true);

  if (guard.exchange(false)) {
    std::vector < std::string > arguments;

    if (argc != 2) {
      _argument.reset(new Argument(true, 1, arguments));
    } else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-H")) {
      _argument.reset(new Argument(true, 0, arguments));
    } else {
      while (--argc) {
        arguments.push_back(argv[argc]);
      }

      _argument.reset(new Argument(false, 0, arguments));
    }

    return true;
  }

  return false; // callers need to "pause and retry" if not serialised ..
}

} // ns util
} // ns ib
