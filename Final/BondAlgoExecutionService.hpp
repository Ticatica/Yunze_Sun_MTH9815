/**
* BondAlgoExecutionService.hpp
* Definition of BondAlgoExecutionService class
*
* 1 Listener: BondExecutionService
*
* @Yunze Sun
*/

#ifndef BondAlgoExecutionService_h
#define BondAlgoExecutionService_h
#include "executionservice.hpp"
#include "BondMarketDataService.hpp"
#include "soa.hpp"
#include "utility.h"
#include <map>
#include <vector>
using namespace std;

class BondAlgoExecutionService : public Service<string, AlgoExecution<Bond> > {
private:
    map<string, AlgoExecution<Bond> > exeMap;
    vector<ServiceListener<AlgoExecution<Bond> >*> listeners;
    static long count;
public:
    // ctor
    BondAlgoExecutionService() { exeMap = map<string, AlgoExecution<Bond> >(); }

    // Implement all the virtual functions

    AlgoExecution<Bond>& GetData(string key) override { return exeMap.at(key); }

    //no need for implementation here
    void OnMessage(AlgoExecution<Bond>& data) override {}

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<AlgoExecution<Bond> >* listener) override {
        listeners.push_back(listener);
    }

    // Get all listeners on the Service.
    const vector<ServiceListener<AlgoExecution<Bond> >*>& GetListeners() const override {
        return listeners;
    }

    // execute algo based on orderbook, called by listener when adding process
    void AlgoTrading(const OrderBook<Bond>& orderBook);
};

long BondAlgoExecutionService::count = 1;

class BondAlgoExecutionServiceListener : public ServiceListener<OrderBook<Bond> > {
public:
    // ctor
    BondAlgoExecutionServiceListener(BondAlgoExecutionService* _bae_service) : bae_service(_bae_service) {};

    // override virtual functions in ServiceListener class
    // listen from BondExecutionService and pass to BondTradeBookingServce
    void ProcessAdd(OrderBook<Bond>& data) override {
        bae_service->AlgoTrading(data);
    };

    // no implementation
    void ProcessRemove(OrderBook<Bond>& data) override {}

    // no implementation
    void ProcessUpdate(OrderBook<Bond>& data) override {}

private:
    BondAlgoExecutionService* bae_service;
};



void BondAlgoExecutionService::AlgoTrading(const OrderBook<Bond>& ob) {
    // get the order book data
    auto bond = ob.GetProduct();
    string id = bond.GetProductId();
    string orderId = "A" + IdGenerator(count,12);

    // get the best bid and offer order and their corresponding price and quantity
    auto bidOffer = ob.GetBestBidOffer();
    Order bid = bidOffer.GetBidOrder();
    Order offer = bidOffer.GetOfferOrder();
    double bidPrice = bid.GetPrice();
    double offerPrice = offer.GetPrice();
    long bidQuantity = bid.GetQuantity();
    long offerQuantity = offer.GetQuantity();

    PricingSide side;
    double price;
    long quantity;

    // only agressing when the spread is at its tightest (1/128)
    if (offerPrice - bidPrice <= 1.0 / 128.0) {
        // alternating between bid and offer, taking the opposite side of the book to cross the spread
        if (count % 2 == 0) {
            side = BID;
            price = offerPrice; // BUY order takes best ask price
            quantity = bidQuantity;
        }
        else {
            side = OFFER;
            price = bidPrice; // SELL order takes best bid price
            quantity = offerQuantity;
        }
    }

    // update the count
    count++;

    // Create the execution order
    // IOC order ¨C immediate-or-cancel
    ExecutionOrder<Bond> executionOrder(bond, side, orderId, IOC, price, quantity, 0, "", false);

    // Create the algo execution
    AlgoExecution<Bond> algoExecution(executionOrder, CME);

    // update the algo execution map
    if (exeMap.find(id) != exeMap.end()) { exeMap.erase(id); }
    exeMap.insert(pair<string, AlgoExecution<Bond>>(id, algoExecution));

    // flow the data to listeners
    for (auto& listener : listeners) {
        listener->ProcessAdd(algoExecution);
    }
}

#endif 