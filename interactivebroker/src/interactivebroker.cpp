//
//  main.cpp
//  interactivebroker
//

// SUMMARY
//
// This code is an interactivebroker client
// More info at https://interactivebrokers.github.io/tws-api/introduction.html
// Complete NYI and TODO in code. NYI's are more immediate. TODO's are more ambitious
// This targets Linux but currently builds and runs on MacOS too.
// Build requires TWS client code and sqlite3 static lib. This repo includes source code from TWS version twsapi_macunix.976.01 in lib and TWS's client example in client dirs 
// 

// TODO:
//
// 1. Performance - code and DB. Put PK back. Use prepared statement. Emplace inserts. Minimal information over pipe. Strip timestamps in some modes or stagger.
// 2. Tidy - move state and exit checks to common code. Reduce 3 signals to 1 or 2.
// 3. Reduce Client class
// 4. Move Const to Configuration/Argument
// 5. Improve DB error handling
// 6. Make pipielines less lossy. Replay
// 7. Reduce or remove Tick class
// 8. Expand threading in line with more detailed Spec/Account info
// 9. Valgrind code
// 10. Document

#include <iostream>
#include <thread>
#include <deque>
#include <csignal>

#include "ib_argument.h"
#include "ib_log.h"
#include "ib_time.h"
#include "ib_timer.h"
#include "ib_client.h"
#include "ib_session.h"

void signal_handler(int signal_num) {
  
  std::cout << "stopping[" << signal_num << "] ..." << std::endl;
  std::cout.flush();

  ib::util::system::threading::_shutdown.store(true);
}

int main(int argc, const char *argv[]) {
  
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);

  std::cout << "starting ..." << std::endl;
  std::cout.flush();

  std::shared_ptr<long long> ms(std::make_shared<long long>(0));

  {
    if (!ib::util::Argument::init(argc, argv)) {
      LOGGER_ERROR("arguments failed");

      ib::util::Argument::help();

      return 1;
    }

    std::vector < std::tuple<int, std::shared_ptr<Contract>> > contracts;

    for (const char *ticker : ib::util::Configuration::configuration()->tickers()) {
      std::shared_ptr < Contract > contract(std::make_shared<Contract>());

      contract->symbol = ticker;
      contract->secType =
          ib::util::Const::_CONST_STOCKCONTRACT_TICKERDEFAULT_SECTYPE_DEFAULT;
      contract->currency =
          ib::util::Const::_CONST_STOCKCONTRACT_TICKERDEFAULT_CURRENCY_DEFAULT;
      contract->exchange =
          ib::util::Const::_CONST_STOCKCONTRACT_TICKERDEFAULT_EXCHANGE_DEFAULT;

      contracts.push_back(std::make_tuple<>(contracts.size(), contract));
    }

    LOGGER_DEBUG("created ", contracts.size(), " oontracts");

    ib::feed::Session session(contracts);

    if (!session.start()) {
      LOGGER_ERROR("could NOT start session");

      return 1;
    }

    while (!ib::util::system::threading::_shutdown && session.ok()) {
      LOGGER_TRACE("session ok");

      if (ib::util::Logging::fatal()) {
        LOGGER_ERROR("fatal event logged so must exit");

        break;
      }

      ib::util::Time::sleep(
          ib::util::Const::_CONST_POLL_CONTROLLER_SLEEP_TIME_MS);
    }

    if (!session.stop()) {
      LOGGER_ERROR("could NOT stop session");

      return 1;
    }
  }

  LOGGER_DEBUG("session completed");

  LOGGER_INFO("runtime: ", *ms, "ms ", *ms / 1000, "s ",
      static_cast<int>((*ms / 1000) / 60), "m", (*ms / 1000) % 60, "s");

  LOGGER_INFO("fini ...");

  return 0;
}
