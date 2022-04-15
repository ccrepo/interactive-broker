//
//  ib_system.cpp
//  interactivebroker
//

#include "ib_system.h"

namespace ib {
namespace util {
namespace system {

namespace threading {
std::atomic<bool> _shutdown(false);
} // ns threading

} // ns system
} // ns util
} // ns ib

