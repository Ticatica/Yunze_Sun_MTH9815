/**
* BondTradeBookingService.hpp
* Definition of BondTradeBookingService class
* 
* 1 Connector
* BondTradeBookingServiceConnector - Read data from "trades.txt"
* 
* 1 Listener
* BondTradeBookingServiceListener - BondTradeBookingService listen from BondExecutionService
* 
* @Yunze Sun
*/

#ifndef BondTradeBookingService_h
#define BondTradeBookingService_h

#include <fstream>
#include "tradebookingservice.hpp"
#include "products.hpp"
#include "BondExecutionService.hpp"
#include "utility.h"

using namespace std;

class BondTradeBookingService : public TradeBookingService<Bond> {
private:
    map<string, Trade<Bond> > trades; // map: id -> Trade
    vector<ServiceListener<Trade<Bond> >*> listeners;

public:
    // Constructor
    BondTradeBookingService() { trades = map<string, Trade<Bond> >(); }

    // Implement the virtual func
    Trade<Bond>& GetData(string key) override { return trades.at(key); }

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Trade<Bond>& data) override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<Trade<Bond> >* listener) override { listeners.push_back(listener); };

    // Get all listeners on the Service.
    const vector<ServiceListener<Trade<Bond> >*>& GetListeners() const override { return listeners; };

    void BookTrade(const Trade<Bond>& trade) override;
};


// Connector for BondTradeBookingService to read "price.txt"
class BondTradeBookingServiceConnector : public Connector<Trade<Bond> > {
private:
    BondTradeBookingService* btb_service;

public:
    // Constructor
    BondTradeBookingServiceConnector(BondTradeBookingService* _btb_service) : btb_service(_btb_service) {};

    // Subscribe-only
    void Publish(Trade<Bond>& data) override {};

    // Subscribe data from price.txt file
    void Subscribe();

};


class BondTradeBookingServiceListener : public ServiceListener<ExecutionOrder<Bond> > {
public:
    // Constructor
    BondTradeBookingServiceListener(BondTradeBookingService* _service) : btb_service(_service) {};

    // override virtual functions in ServiceListener class
    // listen from BondExecutionService and pass to BondTradeBookingServce
    void ProcessAdd(ExecutionOrder<Bond>& data) override;

    // no implementation
    void ProcessRemove(ExecutionOrder<Bond>& data) override {}

    // no implementation
    void ProcessUpdate(ExecutionOrder<Bond>& data) override {}

private:
    BondTradeBookingService* btb_service;
};


// Implemention for BondTradeBookingService class
void BondTradeBookingService::OnMessage(Trade<Bond>& trade) {
    BookTrade(trade);

    for (auto& listener : listeners) {
        listener->ProcessAdd(trade);
    }
}
void BondTradeBookingService::BookTrade(const Trade<Bond>& trade) {
    string id = trade.GetTradeId();
    // Identify if the trade is already recorded
    if (trades.find(id) != trades.end()) { trades.erase(id); }

    trades.insert(pair<string, Trade<Bond> >(id, trade));
}

// Implemention for BondTradeBookingServiceConnector class
void BondTradeBookingServiceConnector::Subscribe() {
    // read data from trades.txt
    ifstream file("trades.txt", ios::in);
    if (file.is_open()) {
        string _line, _data;
        while (getline(file, _line)) {
            stringstream line(_line);
            vector<string> tradeVec;
            while (getline(line, _data, ','))
            {
                tradeVec.push_back(_data);
            }
            // product id
            string productID = tradeVec[0];
            Bond product = GetBond(productID);
            // trade id
            string tradeID = tradeVec[1];
            // price
            double price = Str2Price(tradeVec[2]);
            // book
            string book = tradeVec[3];
            // quantity
            long quantity = stol(tradeVec[4]);
            // side
            Side side = (tradeVec[5] == "BUY" ? BUY : SELL);

            Trade<Bond> new_trade(product, tradeID, price, book, quantity, side);
            // call Service.OnMessage(), flow data
            btb_service->OnMessage(new_trade);

        }
    }
}

void BondTradeBookingServiceListener::ProcessAdd(ExecutionOrder<Bond>& data) {
    auto bond = data.GetProduct();
    double price = data.GetPrice();
    long quantity = data.GetVisibleQuantity();
    string tradeID = "Execution";
    int i = rand() % 3;
    string book;
    switch (i) {
    case 0:
        book = "TRSY1"; break;
    case 1:
        book = "TRSY2"; break;
    case 2:
        book = "TRSY3"; break;
    }
    Side side = (data.GetSide() == BID) ? SELL : BUY;
    Trade<Bond> trade(bond, tradeID, price, book, quantity, side);

    btb_service->OnMessage(trade);   // add to book
}


#endif