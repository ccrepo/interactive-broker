//
// ib_const.h
// interactivebroker
//

#ifndef IB_CONST_H
#define IB_CONST_H

#include <array>
#include <tuple>
#include <map>

namespace ib {
namespace util {

class Const {
public:

  // logging
  static constexpr const char *_CONST_ENVVAR_LOGLEVEL { "IB_LOGLEVEL" };
  static constexpr const char *_CONST_ENVVAR_LOGLEVEL_TRACE { "trace" };
  static constexpr const char *_CONST_ENVVAR_LOGLEVEL_DEBUG { "debug" };
  static constexpr const char *_CONST_ENVVAR_LOGLEVEL_INFO { "info" };
  static constexpr const char *_CONST_ENVVAR_LOGLEVEL_WARN { "warn" };
  static constexpr const char *_CONST_ENVVAR_LOGLEVEL_ERROR { "error" };
  static constexpr const char *_CONST_ENVVAR_LOGLEVEL_FATAL { "fatal" };

  // threading
  static constexpr const unsigned int _CONST_THREADING_SHORTSLEEP { 25 };
  static constexpr const unsigned int _CONST_THREADING_LONGSLEEP { 75 };
  static constexpr const unsigned int _CONST_THREADING_MAXTHREADS { 1 };

  // connection
  static constexpr const char *_CONST_CONNECTION_DEFAULT_HOST { "127.0.0.1" };
  static constexpr const char *_CONST_CONNECTION_DEFAULT_CONNECTOPTIONS { "" };
  static constexpr const unsigned int _CONST_CONNECTION_MAX_ATTEMPTS { 10 };
  static constexpr const unsigned int _CONST_CONNECTION_SLEEP_TIME_MS { 10000 };
  static constexpr const unsigned int _CONST_CONNECTION_DEFAULT_PORT { 7496 };
  static constexpr const unsigned int _CONST_CONNECTION_DEFAULT_CLIENTID { 0 };
  static constexpr const unsigned int _CONST_CONNECTION_DEFAULT_TIMEOUT_MS {
      2000 };
  static constexpr const unsigned int _CONST_CONNECTION_PING_DEADLINE { 2 };
  static constexpr const unsigned int _CONST_CONNECTION_PING_INTRAPING_SLEEP {
      30 };
  static constexpr const unsigned int _CONST_CONNECTION_RETRIES { 2 };
  static constexpr const unsigned int _CONST_CONNECTION_RETRIES_PAUSE_MIN { 100 };
  static constexpr const unsigned int _CONST_CONNECTION_RETRIES_PAUSE_MAX { 200 };
  static constexpr const unsigned int _CONST_CONNECTION_INTRASEQUENCE_PAUSE {
      150 };
  static constexpr const unsigned int _CONST_CONNECTION_MAX_INITIALATTEMPTS { 2 };

  // contract
  static constexpr const char *_CONST_STOCKCONTRACT_TICKERDEFAULT_SECTYPE_DEFAULT {
      "STK" };
  static constexpr const char *_CONST_STOCKCONTRACT_TICKERDEFAULT_CURRENCY_DEFAULT {
      "USD" };
  static constexpr const char *_CONST_STOCKCONTRACT_TICKERDEFAULT_EXCHANGE_DEFAULT {
      "ISLAND" };
  static constexpr const char *_CONST_STOCKCONTRACT_TICKERDEFAULT_TICK {
      "AllLast" };
  static constexpr int _CONST_STOCKCONTRACT_TICKERDEFAULT_NUMTICKS { 0 };
  static constexpr bool _CONST_STOCKCONTRACT_TICKERDEFAULT_IGNORESIZE { true };

  // poll
  static constexpr const unsigned int _CONST_POLL_CONTROLLER_SLEEP_TIME_MS { 200 };
  static constexpr const unsigned int _CONST_POLL_DB_FORCEACTION_LIMIT { 3 };
  static constexpr const unsigned int _CONST_POLL_DB_HEARTBEAT_SLEEP { 1000 };
  static constexpr const long long _CONST_POLL_DB_HEARTBEAT_ALERTTHRESHOLD_TIME {
      60000 };

  // db
  static constexpr const char *_CONST_DATABASE_DEFAULT_DBNAME {
      "interactivebroker.db" };
  //static constexpr const char* _CONST_DATABASE_DEFAULT_USER     { "" };
  //static constexpr const char* _CONST_DATABASE_DEFAULT_PASSWORD { "" };
  //static constexpr const char* _CONST_DATABASE_DEFAULT_SCHEMA { "" };
  static constexpr int _CONST_DATABASE_DEFAULT_RETRYLIMIT { 5 };
  static constexpr int _CONST_DATABASE_DEFAULT_RETRYSLEEP { 2500 };
  // NYI/TODO remove pk constraint from table for testing
  // static constexpr const char* _CONST_DATABASE_DEFAULT_CREATETABLE_SQL { "create table if NOT 
  // exists Ticker (time datetime primary key, price real(15,5), volume integer)" };
  static constexpr const char *_CONST_DATABASE_DEFAULT_CREATETABLE_SQL {
      "create table if NOT exists Ticker (time datetime, price real(15,5), volume integer)" };
  static constexpr int _CONST_DATABASE_DEFAULT_STATEMENT_PAUSE { 25 };

  // test
  // TODO FAKEDATA only for testing. remove this code or at least compile set to falae.
  static constexpr bool _CONST_STOCKCONTRACT_TICKERDEFAULT_FAKEDATA { true };
  static constexpr int _CONST_STOCKCONTRACT_TICKERDEFAULT_FAKEDATA_INSTANCES {
      250 };
  static constexpr const char *_CONST_STOCKCONTRACT_TICKERDEFAULT_FAKEDATA_DATE {
      "2022-02-20 12:34:56" }; // yyyy-MM-dd HH:mm:ss

private:
  Const() = delete;
};

} // ns util
} // ns ib

#endif /* IB_CONST_H */

