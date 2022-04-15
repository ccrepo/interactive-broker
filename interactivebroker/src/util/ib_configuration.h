//
// ib_configuration.h
// interactivebroker
//

#ifndef IB_CONFIGURATION_H
#define IB_CONFIGURATION_H

#include <iostream>
#include <vector>

#include "ib_system.h"
#include "ib_const.h"
#include "ib_value.h"

namespace ib {
namespace util {

class LogFatal;
class LogError;
class LogWarn;
class LogInfo;
class LogDebug;
class LogTrace;

template<typename T> class Logger;

// TODO!!! This needs to read from a file or command line.

class Configuration {

private:
  Configuration() {
  }

  static Configuration *_configuration;

public:
  virtual ~Configuration() {
  }

  static Configuration* configuration() {
    return _configuration;
  }

  inline bool isValid() const {
    return true;
  }

  inline const std::string& level() const {
    static const std::string value(
        util::Value::lowercase(
            util::Value::trim(
                util::system::getenv(util::Const::_CONST_ENVVAR_LOGLEVEL))));

    return value;
  }

  inline bool isLevelTrace() const {
    return this->level() == util::Const::_CONST_ENVVAR_LOGLEVEL_TRACE;
  }

  inline bool isLevelDebug() const {
    return this->level() == util::Const::_CONST_ENVVAR_LOGLEVEL_DEBUG;
  }

  inline bool isLevelInfo() const {
    return this->level() == util::Const::_CONST_ENVVAR_LOGLEVEL_INFO;
  }

  inline bool isLevelWarn() const {
    return this->level() == util::Const::_CONST_ENVVAR_LOGLEVEL_WARN;
  }

  inline bool isLevelError() const {
    return this->level() == util::Const::_CONST_ENVVAR_LOGLEVEL_ERROR;
  }

  inline bool isLevelFatal() const {
    return this->level() == util::Const::_CONST_ENVVAR_LOGLEVEL_FATAL;
  }

  inline constexpr const char* host() const {
    return ib::util::Const::_CONST_CONNECTION_DEFAULT_HOST;
  }

  inline constexpr const char* connectoptions() const {
    return ib::util::Const::_CONST_CONNECTION_DEFAULT_CONNECTOPTIONS;
  }

  inline constexpr int port() const {
    return ib::util::Const::_CONST_CONNECTION_DEFAULT_PORT;
  }

  inline constexpr int clientId() const {
    return ib::util::Const::_CONST_CONNECTION_DEFAULT_CLIENTID;
  }

  inline const std::vector<const char*>& tickers() const {
    //static const std::vector<const char*> tickers { "AFRM", "AGL", "AMC", "APA", "BBWI", "BKR", 
    // "BSY", "BTO", "CCL", "CFG", "CFLT", "CG", "CHWY", "CLF", "CTRA", "CUK", "DISCA", "DT", "EDR", 
    // "EQT", "ET", "F", "FITB", "GM", "HAL", "HBAN", "HOOD", "HPQ", "JNPR", "KEY", "LCID", "LYFT", 
    // "NLOK", "OLPX", "OWL", "PARA", "PATH", "PHM", "PINS", "PLTR", "PLUG", "RF", "RIVN", "RKT", 
    // "S", "SLB", "SNAP", "SYF", "TOST", "TWTR", "UBER", "WDC", "WES", "WFC", "XM", "DFH", "ARHS", 
    // "SOND", "LMAT", "HWKN", "AMSF", "MOV", "SPLP", "LOCL", "KRP", "PTSI", "ACET", "NP", "FC", 
    // "BVH", "FCUV", "BOOM", "PZN", "BTAI", "SBOW" };

    static const std::vector<const char*> tickers { "AFRM" };

    return tickers;
  }

private:
  Configuration(const Configuration&) = delete;
  Configuration& operator=(const Configuration&) = delete;
};

} // ns util
} // ns ib

#endif /* IB_CONFIGURATION_H */
