//
// ib_timer.h
// interactivebroker
//

#ifndef IB_TIMER_H
#define IB_TIMER_H

#include "ib_time.h"

namespace ib {
namespace util {

class Timer {
  
public:
  Timer(const std::shared_ptr<long long> &result) :
      _result(result), _start(ib::util::Time::ts_EPOCH_MS()) {
  }

  virtual ~Timer() {
    if (std::shared_ptr<long long> result = _result.lock()) {
      (*result) = ib::util::Time::ts_EPOCH_MS() - this->_start;
    }
  }

  inline void mark() {
    if (std::shared_ptr<long long> result = _result.lock()) {
      (*result) = ib::util::Time::ts_EPOCH_MS() - this->_start;
    }
  }

private:
  std::weak_ptr<long long> _result;

  const long long _start;

private:
  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;

};

} // ns util
} // ns ib

#endif /* IB_TIMER_H */
