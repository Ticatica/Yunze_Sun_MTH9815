/**
* BondPositionService.hpp
* Definition of PositionService class
*
* 1 Listener:
* BondPositionServiceListener - BondPositionService listen from BondTradeBookingService
*
* @Yunze Sun
*/

#ifndef BondPositionService_h
#define BondPositionService_h

#include "BondTradeBookingService.hpp"
#include "positionservice.hpp"
#include "soa.hpp"

class BondPositionService : public PositionService<Bond> {
public:
    BondPositionService() { positionMap = map<string, Position<Bond> >(); }

    // Implement all the virtual functions

    Position<Bond>& GetData(string key) override { return positionMap.at(key); }

    // No need to implement because there's no linked connector
    void OnMessage(Position<Bond>& data) override {}

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<Position<Bond> >* listener) override {
        listeners.push_back(listener);
    }

    // Get all listeners on the Service.
    const vector<ServiceListener<Position<Bond> >*>& GetListeners() const override {
        return listeners;
    }

    // called by BondPositionServiceListener
    void AddTrade(const Trade<Bond>& trade) override;

private:
    map<string, Position<Bond> > positionMap;
    vector<ServiceListener<Position<Bond> >*> listeners;
};


class BondPositionServiceListener : public ServiceListener<Trade<Bond> > {
private:
    BondPositionService* bpos_service;

public:
    // constructor
    BondPositionServiceListener(BondPositionService* _bpos_service) : bpos_service(_bpos_service) {};

    // Implement all the virtual functions
    void ProcessAdd(Trade<Bond>& data) override { bpos_service->AddTrade(data); };

    // no implementation
    void ProcessRemove(Trade<Bond>& data) override {}

    // no implementation
    void ProcessUpdate(Trade<Bond>& data) override {}

};



// Implement BondPositionService.AddTrade()
void BondPositionService::AddTrade(const Trade<Bond>& trade) 
{
    // get the book
    string book = trade.GetBook();
    // get the id
    auto bond = trade.GetProduct();
    string id = bond.GetProductId();
    
    // new product -> create a pair
    if (positionMap.find(id) == positionMap.end()) {
        positionMap.insert(pair<string, Position<Bond> >(id, Position<Bond>(bond)));
    }

    auto pos = GetData(id);

    // get the quantity
    long delta_position = (trade.GetSide() == BUY) ? trade.GetQuantity() : -trade.GetQuantity();
    // update position
    pos.AddPosition(book, delta_position);

    // inform the listener
    for (auto& listener : listeners) {
        listener->ProcessAdd(pos);
    }
}

#endif