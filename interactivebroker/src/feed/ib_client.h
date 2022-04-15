//
// ib_client.h
// interactivebroker
// thread safe: should be. needs more testing. code is based on TWS API so need to i. check and ii. use carefully.
//

#ifndef IB_CLIENT_H
#define IB_CLIENT_H

#include <memory>
#include <vector>

#include "EWrapper.h"
#include "EReaderOSSignal.h"
#include "EReader.h"

#include "ib_pipe.h"
#include "ib_tick.h"

namespace ib {
namespace feed {

enum State {
  ST_CONNECT,
  ST_TICKDATAOPERATION,
  ST_TICKDATAOPERATION_ACK,
  ST_TICKOPTIONCOMPUTATIONOPERATION,
  ST_TICKOPTIONCOMPUTATIONOPERATION_ACK,
  ST_DELAYEDTICKDATAOPERATION,
  ST_DELAYEDTICKDATAOPERATION_ACK,
  ST_MARKETDEPTHOPERATION,
  ST_MARKETDEPTHOPERATION_ACK,
  ST_REALTIMEBARS,
  ST_REALTIMEBARS_ACK,
  ST_MARKETDATATYPE,
  ST_MARKETDATATYPE_ACK,
  ST_HISTORICALDATAREQUESTS,
  ST_HISTORICALDATAREQUESTS_ACK,
  ST_OPTIONSOPERATIONS,
  ST_OPTIONSOPERATIONS_ACK,
  ST_CONTRACTOPERATION,
  ST_CONTRACTOPERATION_ACK,
  ST_MARKETSCANNERS,
  ST_MARKETSCANNERS_ACK,
  ST_FUNDAMENTALS,
  ST_FUNDAMENTALS_ACK,
  ST_BULLETINS,
  ST_BULLETINS_ACK,
  ST_ACCOUNTOPERATIONS,
  ST_ACCOUNTOPERATIONS_ACK,
  ST_ORDEROPERATIONS,
  ST_ORDEROPERATIONS_ACK,
  ST_OCASAMPLES,
  ST_OCASAMPLES_ACK,
  ST_CONDITIONSAMPLES,
  ST_CONDITIONSAMPLES_ACK,
  ST_BRACKETSAMPLES,
  ST_BRACKETSAMPLES_ACK,
  ST_HEDGESAMPLES,
  ST_HEDGESAMPLES_ACK,
  ST_TESTALGOSAMPLES,
  ST_TESTALGOSAMPLES_ACK,
  ST_FAORDERSAMPLES,
  ST_FAORDERSAMPLES_ACK,
  ST_FAOPERATIONS,
  ST_FAOPERATIONS_ACK,
  ST_DISPLAYGROUPS,
  ST_DISPLAYGROUPS_ACK,
  ST_MISCELANEOUS,
  ST_MISCELANEOUS_ACK,
  ST_CANCELORDER,
  ST_CANCELORDER_ACK,
  ST_FAMILYCODES,
  ST_FAMILYCODES_ACK,
  ST_SYMBOLSAMPLES,
  ST_SYMBOLSAMPLES_ACK,
  ST_REQMKTDEPTHEXCHANGES,
  ST_REQMKTDEPTHEXCHANGES_ACK,
  ST_REQNEWSTICKS,
  ST_REQNEWSTICKS_ACK,
  ST_REQSMARTCOMPONENTS,
  ST_REQSMARTCOMPONENTS_ACK,
  ST_NEWSPROVIDERS,
  ST_NEWSPROVIDERS_ACK,
  ST_REQNEWSARTICLE,
  ST_REQNEWSARTICLE_ACK,
  ST_REQHISTORICALNEWS,
  ST_REQHISTORICALNEWS_ACK,
  ST_REQHEADTIMESTAMP,
  ST_REQHEADTIMESTAMP_ACK,
  ST_REQHISTOGRAMDATA,
  ST_REQHISTOGRAMDATA_ACK,
  ST_REROUTECFD,
  ST_REROUTECFD_ACK,
  ST_MARKETRULE,
  ST_MARKETRULE_ACK,
  ST_PNL,
  ST_PNL_ACK,
  ST_PNLSINGLE,
  ST_PNLSINGLE_ACK,
  ST_CONTFUT,
  ST_CONTFUT_ACK,
  ST_PING,
  ST_PING_ACK,
  ST_REQHISTORICALTICKS,
  ST_REQHISTORICALTICKS_ACK,
  ST_REQTICKBYTICKDATA,
  ST_REQTICKBYTICKDATA_ACK,
  ST_WHATIFSAMPLES,
  ST_WHATIFSAMPLES_ACK,
  ST_IDLE
};

class Subscription;

class Client: public EWrapper // TODO - separate TWS client code and inherit to make upgrades easier
{
  
public:
  Client(const std::shared_ptr<ib::util::Pipe<std::shared_ptr<ib::feed::Tick>>> &pipe);

  virtual ~Client();

  void setConnectOptions(const std::string &connectoptions);

  void processMessages();

  bool connect(const char *host, int port, int clientId,
      Subscription *subscription);

  void disconnect() const;

  bool isConnected() const;

  void fakeTick(int reqId, int tickType, time_t time, double price, int size,
      const TickAttribLast &tickAttribLast, const std::string &exchange,
      const std::string &specialConditions);

private:
  EReaderOSSignal _osSignal;
  std::unique_ptr<EClientSocket> _client;
  State _state;
  time_t _sleepDeadline;
  OrderId _orderId;
  std::unique_ptr<EReader> _reader;
  bool _extraAuth;
  std::string _bboExchange;
  std::shared_ptr<ib::util::Pipe<std::shared_ptr<ib::feed::Tick>>> _pipe;

private:
  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;

private:
  void pnlOperation();
  void pnlSingleOperation();
  void tickDataOperation();
  void tickOptionComputationOperation();
  void delayedTickDataOperation();
  void marketDepthOperations();
  void realTimeBars();
  void marketDataType();
  void historicalDataRequests();
  void optionsOperations();
  void accountOperations();
  void orderOperations();
  void ocaSamples();
  void conditionSamples();
  void bracketSample();
  void hedgeSample();
  void contractOperations();
  void marketScanners();
  void fundamentals();
  void bulletins();
  void testAlgoSamples();
  void financialAdvisorOrderSamples();
  void financialAdvisorOperations();
  void testDisplayGroups();
  void miscelaneous();
  void reqFamilyCodes();
  void reqMatchingSymbols();
  void reqMktDepthExchanges();
  void reqNewsTicks();
  void reqSmartComponents();
  void reqNewsProviders();
  void reqNewsArticle();
  void reqHistoricalNews();
  void reqHeadTimestamp();
  void reqHistogramData();
  void rerouteCFDOperations();
  void marketRuleOperations();
  void continuousFuturesOperations();
  void reqHistoricalTicks();
  void reqTickByTickData();
  void whatIfSamples();
  void reqCurrentTime();

public:
  // events
#include "EWrapper_prototypes.h"

private:
  void printContractMsg(const Contract &contract);
  void printContractDetailsMsg(const ContractDetails &contractDetails);
  void printContractDetailsSecIdList(const TagValueListSPtr &secIdList);
  void printBondContractDetailsMsg(const ContractDetails &contractDetails);

public:
  void streamMktData(int id, const Contract &contract, const std::string &tick,
      int numberOfTicks, bool ignoreSize);

};

} // ns feed
} // ns ib

#endif /* IB_CLIENT_H */
