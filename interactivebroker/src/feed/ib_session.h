//
// ib_sessioon.h
// interactivebroker
//

#ifndef IB_SESSION_H
#define IB_SESSION_H

#include <vector>

#include "ib_subscription.h"

namespace ib {
namespace feed {

class Session {
  
public:
  Session(const std::vector<std::tuple<int, std::shared_ptr<Contract>>> &contracts);

  virtual ~Session();

  bool start();

  bool stop();

  bool ok() const;

  inline bool stopped() const {
    return _stopped;
  }

private:
  std::vector<std::shared_ptr<Subscription>> _subscriptions;
  bool _stopped;

private:
  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;

};

} // ns feed
} // ns ib

#endif /* IB_SESSION_H */
