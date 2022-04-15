//
//  ib_db.hpp
//  interactivebroker
//

#ifndef IB_DB_H
#define IB_DB_H

#include <vector>
#include <string>
#include <queue>
#include <memory>
#include <thread>

#include "sqlite3.h"

#include "ib_pipe.h"
#include "ib_tick.h"
#include "ib_time.h"

namespace ib {
namespace feed {

class Db {
  
public:
  Db(const std::shared_ptr<ib::util::Pipe<std::shared_ptr<ib::feed::Tick>>> &pipe);

  virtual ~Db();

  bool start();

  bool stop();

  static void process(Db *db);

  const std::shared_ptr<ib::util::Pipe<std::shared_ptr<ib::feed::Tick>>> pipe() {
    return _pipe;
  }

  bool ok() const;

private:
  std::atomic<long long> _heartbeat;
  std::shared_ptr<std::thread> _thread;
  std::shared_ptr<ib::util::Pipe<std::shared_ptr<ib::feed::Tick>>> _pipe;
  std::atomic<bool> _exit;

  void beatheart() {
    _heartbeat.store(ib::util::Time::ts_EPOCH_MS());
  }

  bool exit() const {
    return _exit.load();
  }

  // TODO need to expand this out with replay, count since last ok, close() etc
  // sophisticated db interface was not part of spec and sqlite could/should be
  // swapped out for something with more flexibility to get more robust code
  // (OR using code needs more work)
  class Connection {
    
  public:
    Connection(const std::string &dbname);

    virtual ~Connection();

    inline const std::string& dbname() const {
      return _dbname;
    }

    bool open();

    bool exec(long long pk, const std::string &statement);

    static int callback(void *NotUsed, int argc, char **argv,
        char **azColName) {
      // TODO add this to DEBUG or TRACE later
      return 0;
    }

  private:
    const std::string _dbname;
    bool _open;
    sqlite3 *_handle;

    Connection(const Connection &connection);
    Connection& operator=(const Connection &connection);
  };

private:
  Db(const Db&) = delete;
  Db& operator=(const Db&) = delete;

};

} // ns feed
} // ns ib

#endif /* IB_DB_H */
