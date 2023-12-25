/**
* BondAlgoStreamingService.hpp
* Definition of BondAlgoStreamingService class
*
* 1 Listener: BondAlgoStreamingServiceListener
* listen from BondPricingService
*
* @Yunze Sun
*/

#ifndef BondAlgoStreamingService_h
#define BondAlgoStreamingService_h
#include "streamingservice.hpp"
#include "pricingservice.hpp"
#include "soa.hpp"
#include "products.hpp"
#include <map>
#include <vector>
using namespace std;

class BondAlgoStreamingService : public Service<string, AlgoStream<Bond> > {
private:
    map<string, AlgoStream<Bond> > streamMap;
    vector<ServiceListener<AlgoStream<Bond> >*> listeners;
    static long count;

public:
    // ctor
    BondAlgoStreamingService() { streamMap = map<string, AlgoStream<Bond> >(); }

    // Implement all the virtual functions

    AlgoStream<Bond>& GetData(string key) override { return streamMap.at(key); }

    // no need for implementation here
    void OnMessage(AlgoStream<Bond>& data) override {}

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<AlgoStream<Bond> >* listener) override { listeners.push_back(listener); }

    // Get all listeners on the Service.
    const vector<ServiceListener<AlgoStream<Bond> >*>& GetListeners() const override { return listeners; }

    // add price based on orderbook, called by listener when adding process
    void UpdatePrice(const Price<Bond>& price);
};

long BondAlgoStreamingService::count = 1;

class BondAlgoStreamingServiceListener : public ServiceListener<Price<Bond> > {
public:
    // ctor
    BondAlgoStreamingServiceListener(BondAlgoStreamingService* _bas_service) : bas_service(_bas_service) {};

    // override virtual functions in ServiceListener class
    // listen from BondExecutionService and pass to BondTradeBookingServce
    virtual void ProcessAdd(Price<Bond>& data) override { bas_service->UpdatePrice(data); };

    // no implementation
    virtual void ProcessRemove(Price<Bond>& data) override {}

    // no implementation
    virtual void ProcessUpdate(Price<Bond>& data) override {}

private:
    BondAlgoStreamingService* bas_service;
};


void BondAlgoStreamingService::UpdatePrice(const Price<Bond>& price) {
    Bond product = price.GetProduct();
    string id = product.GetProductId();

    double mid = price.GetMid();
    double spread = price.GetBidOfferSpread();
    double bidPrice = mid - spread / 2.0;
    double offerPrice = mid + spread / 2.0;

    // alternate visible size between 1000000 and 2000000
    long visibleQuantity = (count % 2 == 0) ? 1000000 : 2000000;
    // hidden size is twice the visible size
    long hiddenQuantity = visibleQuantity * 2;

    count++;

    // create bid and offer order
    PriceStreamOrder bidOrder(bidPrice, visibleQuantity, hiddenQuantity, BID);
    PriceStreamOrder offerOrder(offerPrice, visibleQuantity, hiddenQuantity, OFFER);
    // create price stream
    PriceStream<Bond> priceStream(product, bidOrder, offerOrder);
    // create algo stream
    AlgoStream<Bond> algoStream(priceStream);

    // update the algo stream map
    if (streamMap.find(id) != streamMap.end()) { streamMap.erase(id); };
    streamMap.insert(pair<string, AlgoStream<Bond>>(id, algoStream));


    for (auto& listener : listeners) {
        listener->ProcessAdd(algoStream);
    }
}

#endif