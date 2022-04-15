//
//  ib_subscription.cpp
//  interactivebroker
//

#include "ib_time.h"
#include "ib_client.h"

#include "ib_subscription.h"

namespace ib {
namespace feed {

Subscription::Subscription(
    const std::vector<std::tuple<int, std::shared_ptr<Contract>>> &contracts) :
    _heartbeat(ib::util::Time::ts_EPOCH_MS()), _thread(), _contracts(), _pipe(
        std::make_shared<ib::util::Pipe<std::shared_ptr<ib::feed::Tick>>>()), _db(
        std::make_unique < ib::feed::Db > (_pipe)), _exit(false) {
  LOGGER_INFO("creating subscription object");

  std::copy(contracts.begin(), contracts.end(), std::back_inserter(_contracts));

  _db->start();
}

Subscription::~Subscription() {
  LOGGER_INFO("destroying subscription object");
}

void Subscription::process(Subscription *subscription) { // TODO - ugky code
  bool success(false);;

  if (subscription == nullptr) {
    LOGGER_FATAL("subscription pointer is NULL");

    return;
  }

  LOGGER_INFO("started subscription thread");

  for (unsigned int attempt(0);; subscription->beatheart()) {
    if (subscription->exit() || ib::util::system::threading::_shutdown) {
      LOGGER_INFO("subscription shutting down");

      return;
    }

    LOGGER_DEBUG("initiating connect attempt ", attempt + 1, " of ",
        ib::util::Const::_CONST_CONNECTION_MAX_ATTEMPTS);

    std::unique_ptr < ib::feed::Client
        > client(new ib::feed::Client(subscription->pipe()));

    if (ib::util::Configuration::configuration()->connectoptions()) {
      client->setConnectOptions(
          ib::util::Configuration::configuration()->connectoptions());
    }

    client->connect(ib::util::Configuration::configuration()->host(),
        ib::util::Configuration::configuration()->port(),
        ib::util::Configuration::configuration()->clientId(), subscription);

    ib::util::Time::sleep(
        ib::util::Const::_CONST_CONNECTION_INTRASEQUENCE_PAUSE);

    if (ib::util::Const::_CONST_STOCKCONTRACT_TICKERDEFAULT_FAKEDATA) // TODO remove when connected to real source
    {
      TickAttribLast attrib { true, true };

      client->fakeTick(1, 1, ib::util::Time::time_t_SECONDS(), 1, 1, attrib,
          "foo", "bar");
    }

    if (client->isConnected()) {
      success = true;

      for (const auto &contract : subscription->contracts()) {
        if (subscription->exit() || ib::util::system::threading::_shutdown) {
          LOGGER_INFO("subscription shutting down");

          return;
        }

        LOGGER_TRACE("creating subscription for contract symbol [",
            std::get < 0 > (contract), "] '", std::get < 1 > (contract)->symbol,
            "'");

        client->streamMktData(std::get < 0 > (contract),
            *std::get < 1 > (contract),
            ib::util::Const::_CONST_STOCKCONTRACT_TICKERDEFAULT_TICK,
            ib::util::Const::_CONST_STOCKCONTRACT_TICKERDEFAULT_NUMTICKS,
            ib::util::Const::_CONST_STOCKCONTRACT_TICKERDEFAULT_IGNORESIZE);

        ib::util::Time::sleep(
            ib::util::Const::_CONST_CONNECTION_INTRASEQUENCE_PAUSE);

      }

      while (client->isConnected()) {
        if (subscription->exit() || ib::util::system::threading::_shutdown) {
          LOGGER_INFO("subscription shutting down");

          return;
        }

        subscription->beatheart();

        LOGGER_TRACE("client is connected");
        LOGGER_TRACE("process messages");

        client->processMessages();
      }

      LOGGER_DEBUG("client connection is lost");
    } else {
      LOGGER_ERROR("client could not connect to host '",
          ib::util::Configuration::configuration()->host(), "' port '",
          ib::util::Configuration::configuration()->port(),
          "' using client ID '",
          ib::util::Configuration::configuration()->clientId(), "'");

      if (!success && attempt >= ib::util::Const::_CONST_CONNECTION_MAX_INITIALATTEMPTS) {

        LOGGER_FATAL("max initial attempts ", attempt ," reached without previous connection success so exit");
        LOGGER_INFO("subscription shutting down");

        return;
      }
    }

    if (++attempt < ib::util::Const::_CONST_CONNECTION_MAX_ATTEMPTS) {
      LOGGER_DEBUG("sleep for ",
          ib::util::Const::_CONST_CONNECTION_SLEEP_TIME_MS,
          " ms before next attempt");

      ib::util::Time::sleep(ib::util::Const::_CONST_CONNECTION_SLEEP_TIME_MS);
    } else {
      break;
    }

    ib::util::Time::sleep(
        ib::util::Const::_CONST_POLL_CONTROLLER_SLEEP_TIME_MS);
  }
}

bool Subscription::start() {
  if (_thread.get()) {
    LOGGER_WARN("thread already started");

    return true;
  }

  this->_thread.reset(new std::thread(ib::feed::Subscription::process, this));
  return true;
}

bool Subscription::stop() {
  LOGGER_INFO("stopping");

  this->_exit.store(true);
  this->_thread->join();

  bool result(this->_db->stop());

  if (result)
  {
    LOGGER_INFO("stopped");
  }
  else
  {
    LOGGER_WARN("stopped [but got error]");
  }

  return result;
}

bool Subscription::ok() const {
  auto ts(_heartbeat.load());

  if (std::abs(ts - ib::util::Time::ts_EPOCH_MS())
      >= ib::util::Const::_CONST_POLL_DB_HEARTBEAT_ALERTTHRESHOLD_TIME) {
    LOGGER_ERROR("heartbeat threshold ",
        ib::util::Const::_CONST_POLL_DB_HEARTBEAT_ALERTTHRESHOLD_TIME,
        " reached");

    LOGGER_ERROR("subscription NOT ok");
    return false;
  }

  if (!this->_db->ok()) {
    return false;
  }

  LOGGER_TRACE("subscription ok");
  return true;
}

} // ns feed
} // ns ib

