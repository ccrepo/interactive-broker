//
// ib_tick.cpp
// interactivebroker
//

#include <sstream>

#include "ib_tick.h"
#include "ib_const.h"

namespace ib {
namespace feed {

Tick::Tick(long long pk, int reqId, int tickType, time_t time, double price,
    int size, bool pastLimit, bool unreported, const std::string &exchange,
    const std::string &specialConditions) :
    _pk(pk), _reqId(reqId), _tickType(tickType), _time(time), _price(price), _size(
        size), _pastLimit(pastLimit), _unreported(unreported), _exchange(
        exchange), _specialConditions(specialConditions) {
}

Tick::Tick(const Tick &tick) :
    _pk(tick._pk), _reqId(tick._reqId), _tickType(tick._tickType), _time(
        tick._time), _price(tick._price), _size(tick._size), _pastLimit(
        tick._pastLimit), _unreported(tick._unreported), _exchange(
        tick._exchange), _specialConditions(tick._specialConditions) {
}

Tick& Tick::operator=(const Tick &tick) {
  _pk = tick._pk;
  _reqId = tick._reqId;
  _tickType = tick._tickType;
  _time = tick._time;
  _price = tick._price;
  _size = tick._size;
  _pastLimit = tick._pastLimit;
  _unreported = tick._unreported;
  _exchange = tick._exchange;
  _specialConditions = tick._specialConditions;

  return (*this);
}

std::string Tick::asSql() const {
  std::ostringstream os;

  os
      << "insert into Ticker (time, price, volume) values " // NYI - only for tests. fix !!!
      << "( '"
      << ib::util::Const::_CONST_STOCKCONTRACT_TICKERDEFAULT_FAKEDATA_DATE
      << "', " << _price << ", " << _size << ")";

  return os.str();
}

} // ns feed
} // ns ib
