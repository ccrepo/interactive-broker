//
// ib_time.h
// interactivebroker
//

#ifndef IB_TIME_H
#define IB_TIME_H

#include <string>
#include <mutex>

#include <chrono>
using namespace std::chrono_literals;

template<typename T1, typename T2>
using mul = std::ratio_multiply<T1, T2>;

#include <ctime>
#include <iomanip>
#include <sstream>
#include <tuple>
#include <cassert>
#include <thread>
#include <string>
#include <atomic>
#include <iostream>
#include <random>

#include "ib_const.h"

namespace ib {
namespace util {

class Time {
  
public:
  inline static std::string timestamp() {
    auto ts { std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now()) };

    std::ostringstream os;

    //os << std::put_time(std::localtime(&ts), "%Y%m%d %H:%M:%S");
    os << std::put_time(std::localtime(&ts), "%H:%M:%S");

    return os.str();
  }

  inline static long long ts_EPOCH_MS() //milliseconds
  {
    static const auto epoch(std::chrono::system_clock::from_time_t(0));

    return std::chrono::duration_cast < std::chrono::milliseconds
        > (std::chrono::system_clock::now() - epoch).count();
  }

  inline static long long ts_EPOCH_US() //microseconds
  {
    static const auto epoch(std::chrono::system_clock::from_time_t(0));

    return std::chrono::duration_cast < std::chrono::microseconds
        > (std::chrono::system_clock::now() - epoch).count();
  }

  inline static time_t time_t_SECONDS() {
    return std::time(NULL);
  }

  inline static void sleep(unsigned int milliseconds) {
    try {
      std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    } catch (...) {
    }
  }

  inline static void sleep(unsigned int lower, unsigned int upper) {
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<> distr(lower, upper);

    try {
      std::this_thread::sleep_for(std::chrono::milliseconds(distr(generator)));
    } catch (...) {
    }
  }

private:
  Time() = delete;

};

} // ns util
} // ns ib

#endif /* IB_TIME_H */
