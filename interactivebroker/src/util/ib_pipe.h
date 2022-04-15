//
//  ib_pipe.hpp
//  interactivebroker
//

#ifndef IB_PIPE_H
#define IB_PIPE_H

#include <atomic>
#include <vector>
#include <string>
#include <queue>
#include <mutex>

namespace ib {
namespace util {

template<typename T>
class Pipe {

public:
  Pipe() :
      _elements(), _mutex() {
  }

  virtual ~Pipe() {
  }

  void push(const T &element) {
    std::unique_lock < std::timed_mutex > lock(_mutex);
    _elements.push(element);
    _empty = false;
  }

  void pop(std::queue<T> &elements) {
    std::unique_lock < std::timed_mutex > lock(_mutex); // only for one consumer one producer so no timeout needed
    elements = std::move(_elements);
    _empty = true;
  }

  bool empty() const {
    return _empty;
  }

private:
  std::queue<T> _elements;
  std::timed_mutex _mutex;
  std::atomic<bool> _empty;

  Pipe(const Pipe&) = delete;
  Pipe& operator=(const Pipe&) = delete;

};

} // ns util
} // ns ib

#endif /* IB_PIPE_H */
