//
//  ib_sessioon.cpp
//  interactivebroker
//

#include <algorithm>

#include "ib_session.h"

namespace ib {
namespace feed {

Session::Session(
    const std::vector<std::tuple<int, std::shared_ptr<Contract>>> &contracts) {
  
  LOGGER_INFO("creating session object");

  const unsigned int whole(
      contracts.size() > ib::util::Const::_CONST_THREADING_MAXTHREADS ?
          static_cast<int>(contracts.size())
              / ib::util::Const::_CONST_THREADING_MAXTHREADS :
          1);

  const unsigned int part(
      contracts.size() > ib::util::Const::_CONST_THREADING_MAXTHREADS ?
          contracts.size() % ib::util::Const::_CONST_THREADING_MAXTHREADS : 0);

  for (unsigned int i(0), j(0), k(0); i < contracts.size(); j = 0) {
    std::vector < std::tuple<int, std::shared_ptr<Contract>> > batch;

    for (; i < contracts.size() && j < whole; i++, j++) {
      batch.push_back(contracts[i]);

      if ((j + 1) == whole && i < contracts.size() && k < part && ++k && ++i) {
        batch.push_back(contracts[i]);
      }
    }

    std::shared_ptr < Subscription
        > subscription(std::make_shared < Subscription > (batch));

    _subscriptions.push_back(subscription);

    LOGGER_DEBUG("created ", _subscriptions.size(), " subscription thread",
        (_subscriptions.size() > 1 ? "s" : ""));
  }
}

Session::~Session() {
  LOGGER_INFO("destroying session object");

  if (_stopped) {
    LOGGER_INFO("session already stopped");

    return;
  }

  LOGGER_INFO("calling stop");

  this->stop();
}

bool Session::start() {
  bool started(false);

  if (!_subscriptions.empty()) {
    started = true;

    for (auto &subscription : _subscriptions) {
      LOGGER_TRACE("starting thread");

      started = subscription->start() && started;

      LOGGER_TRACE("thread started");
    }
  }

  return started;
}

bool Session::stop() {
  if (_stopped) {
    return true;
  }

  std::for_each(_subscriptions.begin(), _subscriptions.end(),
      [](const std::shared_ptr<Subscription> &subscription) {
        return subscription->stop();
      });

  _stopped = true;

  return true; // TODO add RAII set check and background thread kill
}

bool Session::ok() const {
  return std::all_of(_subscriptions.begin(), _subscriptions.end(),
      [](const std::shared_ptr<Subscription> &subscription) {
        return subscription->ok();
      });
}

} // ns feed
} // ns ib
