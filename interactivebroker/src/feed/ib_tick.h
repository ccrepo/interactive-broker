//
// ib_tick.h
// interactivebroker
//

#ifndef IB_TICK_H
#define IB_TICK_H

#include <vector>
#include <string>

namespace ib {
namespace feed {

class Tick {
  
public:
  Tick(long long pk, int reqId, int tickType, time_t time, double price,
      int size, bool pastLimit, bool unreported, const std::string &exchange,
      const std::string &specialConditions);

  Tick(const Tick &tick);

  Tick& operator=(const Tick &tick);

  virtual ~Tick() {
  }

  inline const std::string sql() const // TODO bit expensive
  {
    return this->asSql();
  }

  inline long long pk() const {
    return _pk;
  }

private:
  long long _pk;

  // TWS data - need client spec for more about this. I used defualt values from the TWS client as placeholders
  int _reqId;
  int _tickType;
  time_t _time;
  double _price;
  int _size;
  //const TickAttribLast _tickAttribLast -> struct containing _pastLimit and _unreported. could put in static checks to make sure sizeof and names remain the same.
  bool _pastLimit;
  bool _unreported;
  std::string _exchange;
  std::string _specialConditions;

  std::string asSql() const;
};

} // ns feed
} // ns ib

#endif /* IB_TICK_H */
