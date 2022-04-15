//
//  ib_client.cpp
//  interactivebroker
//

#include <cassert>

#include <stdio.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <ctime>
#include <fstream>
#include <cstdint>

#include "EClientSocket.h"

//#include "EPosixClientSocketPlatform.h"
//#include "Contract.h"
//#include "Order.h"
//#include "OrderState.h"
//#include "Execution.h"
//#include "CommissionReport.h"
//#include "ContractSamples.h"
//#include "OrderSamples.h"
//#include "ScannerSubscription.h"
//#include "ScannerSubscriptionSamples.h"
//#include "executioncondition.h"
//#include "PriceCondition.h"
//#include "MarginCondition.h"
//#include "PercentChangeCondition.h"
//#include "TimeCondition.h"
//#include "VolumeCondition.h"
//#include "AvailableAlgoParams.h"
//#include "FAMethodSamples.h"
//#include "CommonDefs.h"
//#include "AccountSummaryTags.h"
//#include "Utils.h"

#include "ib_client.h"
#include "ib_const.h"
#include "ib_time.h"
#include "ib_log.h"
#include "ib_subscription.h"

namespace ib {
namespace feed {

Client::Client(
    const std::shared_ptr<ib::util::Pipe<std::shared_ptr<ib::feed::Tick>>> &pipe) :
    _osSignal(ib::util::Const::_CONST_CONNECTION_DEFAULT_TIMEOUT_MS), _client(
        new EClientSocket(this, &_osSignal)), _state(ST_CONNECT), _sleepDeadline(
        static_cast<time_t>(0)), _orderId(static_cast<OrderId>(0)), _reader(), _extraAuth(
        false), _pipe(pipe) {
}

Client::~Client() {
}

void Client::setConnectOptions(const std::string &connectoptions) {
  _client->setConnectOptions(connectoptions);
}

// TODO - this is fake just fpr testing. when connected to real TWS source this can be removed.
void Client::fakeTick(int reqId, int tickType, time_t time, double price,
    int size, const TickAttribLast &tickAttribLast, const std::string &exchange,
    const std::string &specialConditions) {
  for (int x = 0;
      x < ib::util::Const::_CONST_STOCKCONTRACT_TICKERDEFAULT_FAKEDATA_INSTANCES;
      x++) {
    this->tickByTickAllLast(reqId, tickType, time, price, size, tickAttribLast,
        exchange, specialConditions);
  }
}

bool Client::connect(const char *host, int port, int clientId,
    Subscription *subscription) {
  if (subscription == nullptr) {
    LOGGER_FATAL("subscription pointer is NULL");

    return false;
  }

  bool result = false;

  for (unsigned int x(0); x < ib::util::Const::_CONST_CONNECTION_RETRIES; x++) {
    if (x) {
      ib::util::Time::sleep(
          ib::util::Const::_CONST_CONNECTION_RETRIES_PAUSE_MIN,
          ib::util::Const::_CONST_CONNECTION_RETRIES_PAUSE_MAX);
    }

    if (subscription->exit() || ib::util::system::threading::_shutdown) {
      LOGGER_INFO("client connect shutting down");

      return false;
    }

    LOGGER_INFO("connecting to host '", host, "' port '", port, "' client ID '",
        clientId, "'");

    result = _client->eConnect(host, port, clientId, _extraAuth);

    if (result) {
      LOGGER_INFO("connected to host '", host, "' port '", port,
          "' client ID '", clientId, "'");

      _reader.reset(new EReader(_client.get(), &_osSignal));

      _reader->start();

      break;
    }

    LOGGER_ERROR("client could not connect to host '", _client->host().c_str(),
        "' port '", _client->port(), "' using client ID '", clientId, "'");
  }

  return result;
}

void Client::disconnect() const {
  _client->eDisconnect();
}

bool Client::isConnected() const {
  return _client->isConnected();
}

void Client::connectAck() {
  if (!_extraAuth && _client->asyncEConnect()) {
    LOGGER_DEBUG("start api");
    _client->startApi();
  }
}

void Client::reqCurrentTime() {
  _sleepDeadline = ib::util::Time::time_t_SECONDS()
      + ib::util::Const::_CONST_CONNECTION_PING_DEADLINE;

  _state = ST_PING_ACK;

  _client->reqCurrentTime();
}

void Client::streamMktData(int id, const Contract &contract,
    const std::string &tick, int numberOfTicks, bool ignoreSize) {
  _client->reqTickByTickData(id, contract, tick, numberOfTicks, ignoreSize);
}

void Client::processMessages() {
  time_t now_SECONDS(ib::util::Time::time_t_SECONDS());

  switch (_state) {
  case ST_PNLSINGLE:
    LOGGER_TRACE("ST_PNLSINGLE");
    pnlSingleOperation();
    break;

  case ST_PNLSINGLE_ACK:
    LOGGER_TRACE("ST_PNLSINGLE_ACK");
    break;

  case ST_PNL:
    LOGGER_TRACE("ST_PNL");
    pnlOperation();
    break;

  case ST_PNL_ACK:
    LOGGER_TRACE("ST_PNL_ACK");
    break;

  case ST_TICKDATAOPERATION:
    LOGGER_TRACE("ST_TICKDATAOPERATION");
    tickDataOperation();
    break;

  case ST_TICKDATAOPERATION_ACK:
    LOGGER_TRACE("ST_TICKDATAOPERATION_ACK");
    break;

  case ST_TICKOPTIONCOMPUTATIONOPERATION:
    LOGGER_TRACE("ST_TICKOPTIONCOMPUTATIONOPERATION");
    tickOptionComputationOperation();
    break;

  case ST_TICKOPTIONCOMPUTATIONOPERATION_ACK:
    LOGGER_TRACE("ST_TICKOPTIONCOMPUTATIONOPERATION_ACK");
    break;

  case ST_DELAYEDTICKDATAOPERATION:
    LOGGER_TRACE("ST_DELAYEDTICKDATAOPERATION");
    delayedTickDataOperation();
    break;

  case ST_DELAYEDTICKDATAOPERATION_ACK:
    LOGGER_TRACE("ST_DELAYEDTICKDATAOPERATION_ACK");
    break;

  case ST_MARKETDEPTHOPERATION:
    LOGGER_TRACE("ST_MARKETDEPTHOPERATION");
    marketDepthOperations();
    break;

  case ST_MARKETDEPTHOPERATION_ACK:
    LOGGER_TRACE("ST_MARKETDEPTHOPERATION_ACK");
    break;

  case ST_REALTIMEBARS:
    LOGGER_TRACE("ST_REALTIMEBARS");
    realTimeBars();
    break;

  case ST_REALTIMEBARS_ACK:
    LOGGER_TRACE("ST_REALTIMEBARS_ACK");
    break;

  case ST_MARKETDATATYPE:
    LOGGER_TRACE("ST_MARKETDATATYPE");
    marketDataType();
    break;

  case ST_MARKETDATATYPE_ACK:
    LOGGER_TRACE("ST_MARKETDATATYPE_ACK");
    break;

  case ST_HISTORICALDATAREQUESTS:
    LOGGER_TRACE("ST_HISTORICALDATAREQUESTS");
    historicalDataRequests();
    break;

  case ST_HISTORICALDATAREQUESTS_ACK:
    LOGGER_TRACE("ST_HISTORICALDATAREQUESTS_ACK");
    break;

  case ST_OPTIONSOPERATIONS:
    LOGGER_TRACE("ST_OPTIONSOPERATIONS");
    optionsOperations();
    break;

  case ST_OPTIONSOPERATIONS_ACK:
    LOGGER_TRACE("ST_OPTIONSOPERATIONS_ACK");
    break;

  case ST_CONTRACTOPERATION:
    LOGGER_TRACE("ST_CONTRACTOPERATION");
    contractOperations();
    break;

  case ST_CONTRACTOPERATION_ACK:
    LOGGER_TRACE("ST_CONTRACTOPERATION_ACK");
    break;

  case ST_MARKETSCANNERS:
    LOGGER_TRACE("ST_MARKETSCANNERS");
    marketScanners();
    break;

  case ST_MARKETSCANNERS_ACK:
    LOGGER_TRACE("ST_MARKETSCANNERS_ACK");
    break;

  case ST_FUNDAMENTALS:
    LOGGER_TRACE("ST_FUNDAMENTALS");
    fundamentals();
    break;

  case ST_FUNDAMENTALS_ACK:
    LOGGER_TRACE("ST_FUNDAMENTALS_ACK");
    break;

  case ST_BULLETINS:
    LOGGER_TRACE("ST_BULLETINS");
    bulletins();
    break;

  case ST_BULLETINS_ACK:
    LOGGER_TRACE("ST_BULLETINS_ACK");
    break;

  case ST_ACCOUNTOPERATIONS:
    LOGGER_TRACE("ST_ACCOUNTOPERATIONS");
    accountOperations();
    break;

  case ST_ACCOUNTOPERATIONS_ACK:
    LOGGER_TRACE("ST_ACCOUNTOPERATIONS_ACK");
    break;

  case ST_ORDEROPERATIONS:
    LOGGER_TRACE("ST_ORDEROPERATIONS");
    orderOperations();
    break;

  case ST_ORDEROPERATIONS_ACK:
    LOGGER_TRACE("ST_ORDEROPERATIONS_ACK");
    break;

  case ST_OCASAMPLES:
    LOGGER_TRACE("ST_OCASAMPLES");
    ocaSamples();
    break;

  case ST_OCASAMPLES_ACK:
    LOGGER_TRACE("ST_OCASAMPLES_ACK");
    break;

  case ST_CONDITIONSAMPLES:
    LOGGER_TRACE("ST_CONDITIONSAMPLES");
    conditionSamples();
    break;

  case ST_CONDITIONSAMPLES_ACK:
    LOGGER_TRACE("ST_CONDITIONSAMPLES_ACK");
    break;

  case ST_BRACKETSAMPLES:
    LOGGER_TRACE("ST_BRACKETSAMPLES");
    bracketSample();
    break;

  case ST_BRACKETSAMPLES_ACK:
    LOGGER_TRACE("ST_BRACKETSAMPLES_ACK");
    break;

  case ST_HEDGESAMPLES:
    LOGGER_TRACE("ST_HEDGESAMPLES");
    hedgeSample();
    break;

  case ST_HEDGESAMPLES_ACK:
    LOGGER_TRACE("ST_HEDGESAMPLES_ACK");
    break;

  case ST_TESTALGOSAMPLES:
    LOGGER_TRACE("ST_TESTALGOSAMPLES");
    testAlgoSamples();
    break;

  case ST_TESTALGOSAMPLES_ACK:
    LOGGER_TRACE("ST_TESTALGOSAMPLES_ACK");
    break;

  case ST_FAORDERSAMPLES:
    LOGGER_TRACE("ST_FAORDERSAMPLES");
    financialAdvisorOrderSamples();
    break;

  case ST_FAORDERSAMPLES_ACK:
    LOGGER_TRACE("ST_FAORDERSAMPLES_ACK");
    break;

  case ST_FAOPERATIONS:
    LOGGER_TRACE("ST_FAOPERATIONS");
    financialAdvisorOperations();
    break;

  case ST_FAOPERATIONS_ACK:
    LOGGER_TRACE("ST_FAOPERATIONS_ACK");
    break;

  case ST_DISPLAYGROUPS:
    LOGGER_TRACE("ST_DISPLAYGROUPS");
    testDisplayGroups();
    break;

  case ST_DISPLAYGROUPS_ACK:
    LOGGER_TRACE("ST_DISPLAYGROUPS_ACK");
    break;

  case ST_MISCELANEOUS:
    LOGGER_TRACE("ST_MISCELANEOUS");
    miscelaneous();
    break;

  case ST_MISCELANEOUS_ACK:
    LOGGER_TRACE("ST_MISCELANEOUS_ACK");
    break;

  case ST_FAMILYCODES:
    LOGGER_TRACE("ST_FAMILYCODES");
    reqFamilyCodes();
    break;

  case ST_FAMILYCODES_ACK:
    LOGGER_TRACE("ST_FAMILYCODES_ACK");
    break;

  case ST_SYMBOLSAMPLES:
    LOGGER_TRACE("ST_SYMBOLSAMPLES");
    reqMatchingSymbols();
    break;

  case ST_SYMBOLSAMPLES_ACK:
    LOGGER_TRACE("ST_SYMBOLSAMPLES_ACK");
    break;

  case ST_REQMKTDEPTHEXCHANGES:
    LOGGER_TRACE("ST_REQMKTDEPTHEXCHANGES");
    reqMktDepthExchanges();
    break;

  case ST_REQMKTDEPTHEXCHANGES_ACK:
    LOGGER_TRACE("ST_REQMKTDEPTHEXCHANGES_ACK");
    break;

  case ST_REQNEWSTICKS:
    LOGGER_TRACE("ST_REQNEWSTICKS");
    reqNewsTicks();
    break;

  case ST_REQNEWSTICKS_ACK:
    LOGGER_TRACE("ST_REQNEWSTICKS_ACK");
    break;

  case ST_REQSMARTCOMPONENTS:
    LOGGER_TRACE("ST_REQSMARTCOMPONENTS");
    reqSmartComponents();
    break;

  case ST_REQSMARTCOMPONENTS_ACK:
    LOGGER_TRACE("ST_REQSMARTCOMPONENTS_ACK");
    break;

  case ST_NEWSPROVIDERS:
    LOGGER_TRACE("ST_NEWSPROVIDERS");
    reqNewsProviders();
    break;

  case ST_NEWSPROVIDERS_ACK:
    LOGGER_TRACE("ST_NEWSPROVIDERS_ACK");
    break;

  case ST_REQNEWSARTICLE:
    LOGGER_TRACE("ST_REQNEWSARTICLE");
    reqNewsArticle();
    break;

  case ST_REQNEWSARTICLE_ACK:
    LOGGER_TRACE("ST_REQNEWSARTICLE_ACK");
    break;

  case ST_REQHISTORICALNEWS:
    LOGGER_TRACE("ST_REQHISTORICALNEWS");
    reqHistoricalNews();
    break;

  case ST_REQHISTORICALNEWS_ACK:
    LOGGER_TRACE("ST_REQHISTORICALNEWS_ACK");
    break;

  case ST_REQHEADTIMESTAMP:
    LOGGER_TRACE("ST_REQHEADTIMESTAMP");
    reqHeadTimestamp();
    break;

  case ST_REQHISTOGRAMDATA:
    LOGGER_TRACE("ST_REQHISTOGRAMDATA");
    reqHistogramData();
    break;

  case ST_REROUTECFD:
    LOGGER_TRACE("ST_REROUTECFD");
    rerouteCFDOperations();
    break;

  case ST_MARKETRULE:
    LOGGER_TRACE("ST_MARKETRULE");
    marketRuleOperations();
    break;

  case ST_CONTFUT:
    LOGGER_TRACE("ST_CONTFUT");
    continuousFuturesOperations();
    break;

  case ST_REQHISTORICALTICKS:
    LOGGER_TRACE("ST_REQHISTORICALTICKS");
    reqHistoricalTicks();
    break;

  case ST_REQHISTORICALTICKS_ACK:
    LOGGER_TRACE("ST_REQHISTORICALTICKS_ACK");
    break;

  case ST_REQTICKBYTICKDATA:
    LOGGER_TRACE("ST_REQTICKBYTICKDATA");
    reqTickByTickData();
    break;

  case ST_REQTICKBYTICKDATA_ACK:
    LOGGER_TRACE("ST_REQTICKBYTICKDATA_ACK");
    break;

  case ST_WHATIFSAMPLES:
    LOGGER_TRACE("ST_WHATIFSAMPLES");
    whatIfSamples();
    break;

  case ST_WHATIFSAMPLES_ACK:
    LOGGER_TRACE("ST_WHATIFSAMPLES_ACK");
    break;

  case ST_PING:
    LOGGER_TRACE("ST_PING");
    reqCurrentTime();
    break;

  case ST_PING_ACK:
    LOGGER_TRACE("ST_PING_ACK");
    if (_sleepDeadline < now_SECONDS) {
      disconnect();
      return;
    }
    break;

  case ST_IDLE:
    LOGGER_TRACE("ST_IDLE");
    if (_sleepDeadline < now_SECONDS) {
      _state = ST_PING;
      return;
    }
    break;

  case ST_CONNECT:
    LOGGER_TRACE("ST_CONNECT");
    break;

  case ST_CANCELORDER:
    LOGGER_TRACE("ST_CANCELORDER");
    break;

  case ST_CANCELORDER_ACK:
    LOGGER_TRACE("ST_CANCELORDER_ACK");
    break;

  case ST_REQHEADTIMESTAMP_ACK:
    LOGGER_TRACE("ST_REQHEADTIMESTAMP_ACK");
    break;

  case ST_REQHISTOGRAMDATA_ACK:
    LOGGER_TRACE("ST_REQHISTOGRAMDATA_ACK");
    break;

  case ST_REROUTECFD_ACK:
    LOGGER_TRACE("ST_REROUTECFD_ACK");
    break;

  case ST_MARKETRULE_ACK:
    LOGGER_TRACE("ST_MARKETRULE_ACK");
    break;

  case ST_CONTFUT_ACK:
    LOGGER_TRACE("ST_CONTFUT_ACK");
    break;
  }

  _osSignal.waitForSignal();

  errno = 0;

  _reader->processMsgs();
}

//

void Client::pnlOperation() {
  _state = ST_PNL_ACK;
}

void Client::pnlSingleOperation() {
  _state = ST_PNLSINGLE_ACK;
}

void Client::tickDataOperation() {
  _state = ST_TICKDATAOPERATION_ACK;
}

void Client::tickOptionComputationOperation() {
  _state = ST_TICKOPTIONCOMPUTATIONOPERATION_ACK;
}

void Client::delayedTickDataOperation() {
  _state = ST_DELAYEDTICKDATAOPERATION_ACK;
}

void Client::marketDepthOperations() {
  _state = ST_MARKETDEPTHOPERATION_ACK;
}

void Client::realTimeBars() {
  _state = ST_REALTIMEBARS_ACK;
}

void Client::marketDataType() {
  _state = ST_MARKETDATATYPE_ACK;
}

void Client::historicalDataRequests() {
  _state = ST_HISTORICALDATAREQUESTS_ACK;
}

void Client::optionsOperations() {
  _state = ST_OPTIONSOPERATIONS_ACK;
}

void Client::contractOperations() {
  _state = ST_CONTRACTOPERATION_ACK;
}

void Client::marketScanners() {
  _state = ST_MARKETSCANNERS_ACK;
}

void Client::fundamentals() {
  _state = ST_FUNDAMENTALS_ACK;
}

void Client::bulletins() {
  _state = ST_BULLETINS_ACK;
}

void Client::accountOperations() {
  _state = ST_ACCOUNTOPERATIONS_ACK;
}

void Client::orderOperations() {
  _state = ST_ORDEROPERATIONS_ACK;
}

void Client::ocaSamples() {
  _state = ST_OCASAMPLES_ACK;
}

void Client::conditionSamples() {
  _state = ST_CONDITIONSAMPLES_ACK;
}

void Client::bracketSample() {
  _state = ST_BRACKETSAMPLES_ACK;
}

void Client::hedgeSample() {
  _state = ST_HEDGESAMPLES_ACK;
}

void Client::testAlgoSamples() {
  _state = ST_TESTALGOSAMPLES_ACK;
}

void Client::financialAdvisorOrderSamples() {
  _state = ST_FAORDERSAMPLES_ACK;
}

void Client::financialAdvisorOperations() {
  _state = ST_FAOPERATIONS_ACK;
}

void Client::testDisplayGroups() {
  _state = ST_TICKDATAOPERATION_ACK;
}

void Client::miscelaneous() {
  _state = ST_MISCELANEOUS_ACK;
}

void Client::reqFamilyCodes() {
  _state = ST_FAMILYCODES_ACK;
}

void Client::reqMatchingSymbols() {
  _state = ST_SYMBOLSAMPLES_ACK;
}

void Client::reqMktDepthExchanges() {
  _state = ST_REQMKTDEPTHEXCHANGES_ACK;
}

void Client::reqNewsTicks() {
  _state = ST_REQNEWSTICKS_ACK;
}

void Client::reqSmartComponents() {
  // TODO state missing?
}

void Client::reqNewsProviders() {
  _state = ST_NEWSPROVIDERS_ACK;
}

void Client::reqNewsArticle() {
  _state = ST_REQNEWSARTICLE_ACK;
}

void Client::reqHistoricalNews() {
  _state = ST_REQHISTORICALNEWS_ACK;
}

void Client::reqHeadTimestamp() {
  _state = ST_REQHEADTIMESTAMP_ACK;
}

void Client::reqHistogramData() {
  _state = ST_REQHISTOGRAMDATA_ACK;
}

void Client::rerouteCFDOperations() {
  _state = ST_REROUTECFD_ACK;
}

void Client::marketRuleOperations() {
  _state = ST_MARKETRULE_ACK;
}

void Client::continuousFuturesOperations() {
  _state = ST_CONTFUT_ACK;
}

void Client::reqHistoricalTicks() {
  _state = ST_REQHISTORICALTICKS_ACK;
}

void Client::reqTickByTickData() {
  _state = ST_REQTICKBYTICKDATA_ACK;
}

void Client::whatIfSamples() {
  _state = ST_WHATIFSAMPLES_ACK;
}

void Client::nextValidId(OrderId orderId) {
  LOGGER_DEBUG("next valid Id is ", orderId);

  _orderId = orderId;

  //_state = ST_TICKOPTIONCOMPUTATIONOPERATION;
  //_state = ST_TICKDATAOPERATION;
  //_state = ST_REQTICKBYTICKDATA;
  //_state = ST_REQHISTORICALTICKS;
  //_state = ST_CONTFUT;
  //_state = ST_PNLSINGLE;
  //_state = ST_PNL;
  //_state = ST_DELAYEDTICKDATAOPERATION;
  //_state = ST_MARKETDEPTHOPERATION;
  //_state = ST_REALTIMEBARS;
  //_state = ST_MARKETDATATYPE;
  //_state = ST_HISTORICALDATAREQUESTS;
  //_state = ST_CONTRACTOPERATION;
  //_state = ST_MARKETSCANNERS;
  //_state = ST_FUNDAMENTALS;
  //_state = ST_BULLETINS;
  //_state = ST_ACCOUNTOPERATIONS;

  _state = ST_ORDEROPERATIONS;

  //_state = ST_OCASAMPLES;
  //_state = ST_CONDITIONSAMPLES;
  //_state = ST_BRACKETSAMPLES;
  //_state = ST_HEDGESAMPLES;
  //_state = ST_TESTALGOSAMPLES;
  //_state = ST_FAORDERSAMPLES;
  //_state = ST_FAOPERATIONS;
  //_state = ST_DISPLAYGROUPS;
  //_state = ST_MISCELANEOUS;
  //_state = ST_FAMILYCODES;
  //_state = ST_SYMBOLSAMPLES;
  //_state = ST_REQMKTDEPTHEXCHANGES;
  //_state = ST_REQNEWSTICKS;
  //_state = ST_REQSMARTCOMPONENTS;
  //_state = ST_NEWSPROVIDERS;
  //_state = ST_REQNEWSARTICLE;
  //_state = ST_REQHISTORICALNEWS;
  //_state = ST_REQHEADTIMESTAMP;
  //_state = ST_REQHISTOGRAMDATA;
  //_state = ST_REROUTECFD;
  //_state = ST_MARKETRULE;
  //_state = ST_PING;
  //_state = ST_WHATIFSAMPLES;
}

void Client::currentTime(long time) {
  if (_state == ST_PING_ACK) {
    const time_t t(static_cast<time_t>(time));

    struct tm *timeinfo = localtime(&t);

    LOGGER_DEBUG("the current date/time is ", asctime(timeinfo));

    time_t now = ib::util::Time::time_t_SECONDS();

    _sleepDeadline = now
        + ib::util::Const::_CONST_CONNECTION_PING_INTRAPING_SLEEP;

    _state = ST_PING_ACK;
  }
}

//

void Client::error(int id, int errorCode, const std::string &errorString) {
  LOGGER_ERROR("error Id '", id, "' Code '", errorCode, "' Msg '",
      errorString.c_str(), "'");
}

//

void Client::tickPrice(TickerId tickerId, TickType field, double price,
    const TickAttrib &attribs) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::tickSize(TickerId tickerId, TickType field, int size) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::tickOptionComputation(TickerId tickerId, TickType tickType,
    double impliedVol, double delta, double optPrice, double pvDividend,
    double gamma, double vega, double theta, double undPrice) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::tickGeneric(TickerId tickerId, TickType tickType, double value) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::tickString(TickerId tickerId, TickType tickType,
    const std::string &value) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::tickEFP(TickerId tickerId, TickType tickType, double basisPoints,
    const std::string &formattedBasisPoints, double totalDividends,
    int holdDays, const std::string &futureLastTradeDate, double dividendImpact,
    double dividendsToLastTradeDate) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::orderStatus(OrderId orderId, const std::string &status,
    double filled, double remaining, double avgFillPrice, int permId,
    int parentId, double lastFillPrice, int clientId,
    const std::string &whyHeld, double mktCapPrice) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::openOrder(OrderId orderId, const Contract &contract,
    const Order &order, const OrderState &orderState) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::openOrderEnd() {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::winError(const std::string &str, int lastError) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::connectionClosed() {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::updateAccountValue(const std::string &key, const std::string &val,
    const std::string &currency, const std::string &accountName) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::updatePortfolio(const Contract &contract, double position,
    double marketPrice, double marketValue, double averageCost,
    double unrealizedPNL, double realizedPNL, const std::string &accountName) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::updateAccountTime(const std::string &timeStamp) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::accountDownloadEnd(const std::string &accountName) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::contractDetails(int reqId,
    const ContractDetails &contractDetails) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::bondContractDetails(int reqId,
    const ContractDetails &contractDetails) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::printContractMsg(const Contract &contract) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::printContractDetailsMsg(const ContractDetails &contractDetails) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::printContractDetailsSecIdList(const TagValueListSPtr &secIdList) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::printBondContractDetailsMsg(
    const ContractDetails &contractDetails) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::contractDetailsEnd(int reqId) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::execDetails(int reqId, const Contract &contract,
    const Execution &execution) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::execDetailsEnd(int reqId) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::updateMktDepth(TickerId id, int position, int operation, int side,
    double price, int size) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::updateMktDepthL2(TickerId id, int position,
    const std::string &marketMaker, int operation, int side, double price,
    int size, bool isSmartDepth) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::updateNewsBulletin(int msgId, int msgType,
    const std::string &newsMessage, const std::string &originExch) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::managedAccounts(const std::string &accountsList) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::receiveFA(faDataType pFaDataType, const std::string &cxml) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::historicalData(TickerId reqId, const Bar &bar) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::historicalDataEnd(int reqId, const std::string &startDateStr,
    const std::string &endDateStr) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::scannerParameters(const std::string &xml) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::scannerData(int reqId, int rank,
    const ContractDetails &contractDetails, const std::string &distance,
    const std::string &benchmark, const std::string &projection,
    const std::string &legsStr) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::scannerDataEnd(int reqId) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::realtimeBar(TickerId reqId, long time, double open, double high,
    double low, double close, long volume, double wap, int count) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::fundamentalData(TickerId reqId, const std::string &data) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::deltaNeutralValidation(int reqId,
    const DeltaNeutralContract &deltaNeutralContract) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::tickSnapshotEnd(int reqId) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::marketDataType(TickerId reqId, int marketDataType) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::commissionReport(const CommissionReport &commissionReport) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::position(const std::string &account, const Contract &contract,
    double position, double avgCost) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::positionEnd() {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::accountSummary(int reqId, const std::string &account,
    const std::string &tag, const std::string &value,
    const std::string &currency) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::accountSummaryEnd(int reqId) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::verifyMessageAPI(const std::string &apiData) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::verifyCompleted(bool isSuccessful, const std::string &errorText) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::verifyAndAuthMessageAPI(const std::string &apiDatai,
    const std::string &xyzChallenge) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::verifyAndAuthCompleted(bool isSuccessful,
    const std::string &errorText) {
  LOGGER_WARN("in");

  if (isSuccessful)
    _client->startApi();

  LOGGER_WARN("out");
}

void Client::displayGroupList(int reqId, const std::string &groups) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::displayGroupUpdated(int reqId, const std::string &contractInfo) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::positionMulti(int reqId, const std::string &account,
    const std::string &modelCode, const Contract &contract, double pos,
    double avgCost) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::positionMultiEnd(int reqId) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::accountUpdateMulti(int reqId, const std::string &account,
    const std::string &modelCode, const std::string &key,
    const std::string &value, const std::string &currency) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::accountUpdateMultiEnd(int reqId) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::securityDefinitionOptionalParameter(int reqId,
    const std::string &exchange, int underlyingConId,
    const std::string &tradingClass, const std::string &multiplier,
    const std::set<std::string> &expirations, const std::set<double> &strikes) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::securityDefinitionOptionalParameterEnd(int reqId) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::softDollarTiers(int reqId,
    const std::vector<SoftDollarTier> &tiers) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::familyCodes(const std::vector<FamilyCode> &familyCodes) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::symbolSamples(int reqId,
    const std::vector<ContractDescription> &contractDescriptions) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::mktDepthExchanges(
    const std::vector<DepthMktDataDescription> &depthMktDataDescriptions) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::tickNews(int tickerId, time_t timeStamp,
    const std::string &providerCode, const std::string &articleId,
    const std::string &headline, const std::string &extraData) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::smartComponents(int reqId, const SmartComponentsMap &theMap) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::tickReqParams(int tickerId, double minTick,
    const std::string &bboExchange, int snapshotPermissions) {
  LOGGER_WARN("in");

  LOGGER_WARN("tickerId '", tickerId, "' minTick '", minTick, "' bboExchange '",
      bboExchange, "' snapshotPermissions '", snapshotPermissions, "'");

  _bboExchange = bboExchange;

  LOGGER_WARN("out");
}

void Client::newsProviders(const std::vector<NewsProvider> &newsProviders) {
  LOGGER_INFO("print function NOT implemented - TODO");
}

void Client::newsArticle(int requestId, int articleType,
    const std::string &articleText) {
  // TODO - print function missing?
}

void Client::historicalNews(int requestId, const std::string &time,
    const std::string &providerCode, const std::string &articleId,
    const std::string &headline) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::historicalNewsEnd(int requestId, bool hasMore) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::headTimestamp(int reqId, const std::string &headTimestamp) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::histogramData(int reqId, const HistogramDataVector &data) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::historicalDataUpdate(TickerId reqId, const Bar &bar) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::rerouteMktDataReq(int reqId, int conid,
    const std::string &exchange) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::rerouteMktDepthReq(int reqId, int conid,
    const std::string &exchange) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::marketRule(int marketRuleId,
    const std::vector<PriceIncrement> &priceIncrements) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::pnl(int reqId, double dailyPnL, double unrealizedPnL,
    double realizedPnL) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::pnlSingle(int reqId, int pos, double dailyPnL,
    double unrealizedPnL, double realizedPnL, double value) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::historicalTicks(int reqId,
    const std::vector<HistoricalTick> &ticks, bool done) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::historicalTicksBidAsk(int reqId,
    const std::vector<HistoricalTickBidAsk> &ticks, bool done) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::historicalTicksLast(int reqId,
    const std::vector<HistoricalTickLast> &ticks, bool done) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::tickByTickAllLast(int reqId, int tickType, time_t time,
    double price, int size, const TickAttribLast &tickAttribLast,
    const std::string &exchange, const std::string &specialConditions) {
  _pipe->push(
      std::shared_ptr < ib::feed::Tick
          > (std::make_shared < ib::feed::Tick
              > (ib::util::Time::ts_EPOCH_US(), reqId, tickType, time, price, size, tickAttribLast.pastLimit, tickAttribLast.unreported, exchange, specialConditions)));
}

void Client::tickByTickBidAsk(int reqId, time_t time, double bidPrice,
    double askPrice, int bidSize, int askSize,
    const TickAttribBidAsk &tickAttribBidAsk) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::tickByTickMidPoint(int reqId, time_t time, double midPoint) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::orderBound(long long orderId, int apiClientId, int apiOrderId) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::completedOrder(const Contract &contract, const Order &order,
    const OrderState &orderState) {
  LOGGER_WARN("print function NOT implemented - TODO");
}

void Client::completedOrdersEnd() {
  LOGGER_WARN("print function NOT implemented - TODO");
}

} // ns feed
} // ns ib
