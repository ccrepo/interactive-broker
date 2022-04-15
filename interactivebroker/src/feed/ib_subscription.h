//
// ib_subscription.h
// interactivebroker
//

#ifndef IB_SUBSCRIPTION_H
#define IB_SUBSCRIPTION_H

#include <thread>
#include <atomic>

#include "ib_client.h"
#include "ib_log.h"
#include "ib_pipe.h"
#include "ib_tick.h"
#include "ib_db.h"
#include "ib_time.h"

namespace ib {
namespace feed {

class Subscription {
  
public:
  Subscription(
      const std::vector<std::tuple<int, std::shared_ptr<Contract>>> &contracts);

  virtual ~Subscription();

  bool start();

  bool stop();

  static void process(Subscription *subscription);

  bool ok() const;

  bool exit() const {
    return _exit.load();
  }

private:
  std::atomic<long long> _heartbeat;
  std::shared_ptr<std::thread> _thread;
  std::vector<std::tuple<int, std::shared_ptr<Contract>>> _contracts;
  std::shared_ptr<ib::util::Pipe<std::shared_ptr<ib::feed::Tick>>> _pipe;
  std::shared_ptr<ib::feed::Db> _db;
  std::atomic<bool> _exit;

  void beatheart() {
    _heartbeat.store(ib::util::Time::ts_EPOCH_MS());
  }

  const std::shared_ptr<ib::util::Pipe<std::shared_ptr<ib::feed::Tick>>> pipe() {
    return _pipe;
  }

  inline const std::vector<std::tuple<int, std::shared_ptr<Contract>>>& contracts() const {
    return _contracts;
  }

private:
  Subscription(const Subscription&) = delete;
  Subscription& operator=(const Subscription&) = delete;
};

} // ns feed
} // ns ib

#endif /* IB_SUBSCRIPTION_H */
