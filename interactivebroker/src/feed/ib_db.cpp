//
//  ib_db.cpp
//  interactivebroker
//

#include "ib_db.h"
#include "ib_log.h"
#include "ib_time.h"

namespace ib {
namespace feed {

Db::Db(const std::shared_ptr<ib::util::Pipe<std::shared_ptr<ib::feed::Tick>>> &pipe) :
    _heartbeat(ib::util::Time::ts_EPOCH_MS()), _thread(), _pipe(pipe), _exit(
        false) {
  LOGGER_INFO("creating db object");
}

Db::~Db() {
  LOGGER_INFO("destroying db object");
}

bool Db::start() {
  if (_thread.get()) {
    LOGGER_WARN("thread already started");

    return true;
  }

  this->_thread.reset(new std::thread(ib::feed::Db::process, this));

  return true;
}

bool Db::stop() {
  LOGGER_INFO("stopping");

  this->_exit.store(true);

  this->_thread->join();

  LOGGER_INFO("stopped");

  return true;
}

void Db::process(Db *db) {
  LOGGER_INFO("started db thread");

  if (db == nullptr) {
    LOGGER_FATAL("db pointer is NULL");

    return;
  }

  ib::feed::Db::Connection connection(
      ib::util::Const::_CONST_DATABASE_DEFAULT_DBNAME);

  try {
    std::queue < std::shared_ptr < ib::feed::Tick >> ticks;

    for (;; db->beatheart()) {
      if (db->exit() || ib::util::system::threading::_shutdown) {
        LOGGER_TRACE("db shutting down");

        return;
      }

      {
        int attempts(0);

        while (!connection.open()) {
          LOGGER_ERROR("DB connection not opened");

          if (db->exit() || ib::util::system::threading::_shutdown) {
            LOGGER_TRACE("db shutting down");

            return;
          }

          attempts++;

          if (attempts >= ib::util::Const::_CONST_DATABASE_DEFAULT_RETRYLIMIT) {
            LOGGER_FATAL("DB connection could NOT be opened - terminating");

            return;
          }

          ib::util::Time::sleep(
              ib::util::Const::_CONST_DATABASE_DEFAULT_RETRYSLEEP);
        }
      }

      if (db->exit() || ib::util::system::threading::_shutdown) {
        LOGGER_TRACE("db shutting down");

        return;
      }

      if (!connection.exec(0,
          ib::util::Const::_CONST_DATABASE_DEFAULT_CREATETABLE_SQL)) {
        // warn and continue
        LOGGER_WARN("db error running sql '",
            ib::util::Const::_CONST_DATABASE_DEFAULT_CREATETABLE_SQL, "'");
      }

      LOGGER_TRACE("checking ticks in pipe");

      if (!db->pipe()->empty()) {
        LOGGER_TRACE("fetching ticks from pipe");

        db->pipe()->pop(ticks);

        LOGGER_TRACE("got ", ticks.size(), " ticks from pipe");

        while (!ticks.empty()) {
          if (db->exit() || ib::util::system::threading::_shutdown) {
            LOGGER_TRACE("db shutting down");

            return;
          }

          db->beatheart();

          const auto &tick(ticks.front());

          LOGGER_DEBUG("running tick sql pk '", tick->pk(), "'");

          if (!connection.exec(tick->pk(), tick->sql())) // TODO bulk apply these to spped up
              {
            // error and continue. 
            // TODO add flag if we want an exit on error
            LOGGER_TRACE("error running tick sql pk '", tick->pk(), "'");
          }

          ticks.pop();

          ib::util::Time::sleep(
              ib::util::Const::_CONST_DATABASE_DEFAULT_STATEMENT_PAUSE); // TODO remove/reduce this based on tests     
        }
      } else {
        LOGGER_TRACE("pipe was empty");
      }

      ib::util::Time::sleep(
          ib::util::Const::_CONST_POLL_CONTROLLER_SLEEP_TIME_MS);
    }
  } catch (...) {
    LOGGER_WARN("unknown exception");
  }
}

bool Db::ok() const {
  auto ts(_heartbeat.load());

  if (std::abs(ts - ib::util::Time::ts_EPOCH_MS())
      >= ib::util::Const::_CONST_POLL_DB_HEARTBEAT_ALERTTHRESHOLD_TIME) {
    LOGGER_ERROR("heartbeat threshold ",
        ib::util::Const::_CONST_POLL_DB_HEARTBEAT_ALERTTHRESHOLD_TIME,
        " reached");

    LOGGER_ERROR("db NOT ok");

    return false;
  }

  LOGGER_TRACE("db ok");

  return true;
}

Db::Connection::Connection(const std::string &dbname) :
    _dbname(dbname), _open(false), _handle(static_cast<sqlite3*>(nullptr)) {
  LOGGER_INFO("creating connection to '", _dbname, "'");
}

Db::Connection::~Connection() {
  LOGGER_INFO("destroying connection to '", _dbname, "'");

  sqlite3_close (_handle);

  _handle = static_cast<sqlite3*>(nullptr);
}

bool Db::Connection::open() // TODO make more robust. retry/reopen
{
  if (!_open) {
    if (sqlite3_open(_dbname.c_str(), &_handle) == SQLITE_OK) {
      LOGGER_INFO("opened connection to '", _dbname, "'");

      _open = true;
    } else {
      if (_handle != nullptr) {
        LOGGER_ERROR("unable to open connection to sqlite dbname '", _dbname,
            "' error code '", sqlite3_errcode(_handle), "' msg '",
            sqlite3_errmsg(_handle), "'");
      } else {
        LOGGER_ERROR("unable to open connection to sqlite dbname '", _dbname,
            "' NULL pointer returned");
      }

      sqlite3_close (_handle);

      _handle = static_cast<sqlite3*>(nullptr);
    }
  } else {
    LOGGER_TRACE("connection to '", _dbname, "' previosuly opened");
  }

  return _open;
}

bool Db::Connection::exec(long long pk, const std::string &statement) {
  char *error(static_cast<char*>(0));

  if (sqlite3_exec(_handle, statement.c_str(),
      ib::feed::Db::Connection::callback, 0, &error) != SQLITE_OK) {
    LOGGER_TRACE("error running sql pk '", pk, " '", statement, "' msg '",
        (error ? error : "NULL"), "'");

    sqlite3_free(error);

    return false;
  }

  return true;
}

} // ns feed
} // ns ib
