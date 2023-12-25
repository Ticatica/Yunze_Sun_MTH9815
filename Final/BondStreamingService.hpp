/**
* BondStreamingService.hpp
* Definition of BondStreamingService class
*
* 1 Connector: BondStreamingServiceConnector
* Publish the price
*
*
* 1 Listener: BondStreamingServiceListener
* listen from BondAlgoStreamingService
*
* @Yunze Sun
*/

#ifndef BondStreamingService_h
#define BondStreamingService_h

#include "streamingservice.hpp"
#include "BondAlgoStreamingService.hpp"

class BondStreamingServiceConnector;

class BondStreamingService : public StreamingService<Bond> {
private:
    map<string, PriceStream<Bond> > streamMap;
    vector<ServiceListener<PriceStream<Bond> >*> listeners;
    BondStreamingServiceConnector* conn;

public:
    // ctor
    BondStreamingService(BondStreamingServiceConnector* _conn) : conn(_conn) { streamMap = map<string, PriceStream<Bond> >(); };

    // Implement all the virtual functions

    PriceStream<Bond>& GetData(string key) override { return streamMap.at(key); }

    // no need for implementation here
    void OnMessage(PriceStream<Bond>& data) override {}

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<PriceStream<Bond> >* listener) override { listeners.push_back(listener); }

    // Get all listeners on the Service.
    const vector<ServiceListener<PriceStream<Bond> >*>& GetListeners() const override { return listeners; }

    // publish the price via connector
    void PublishPrice(PriceStream<Bond>& price_stream);

    // called by BondStreamingServiceListener
    void UpdateStream(const AlgoStream<Bond>& algo);
};


class BondStreamingServiceConnector : public Connector<PriceStream<Bond> > {
public:
    // ctor
    BondStreamingServiceConnector() {}

    // override pure virtual functions in base class
    // publish ExecutionOrder, called by BondExecutionService
    void Publish(PriceStream<Bond>& data) override;

    // publish only, no subsribe
    void Subscribe() {}
};



class BondStreamingServiceListener : public ServiceListener<AlgoStream<Bond> > {
public:
    // ctor
    BondStreamingServiceListener(BondStreamingService* _bs_service) : bs_service(_bs_service) {}

    // override virtual functions in ServiceListener class
    void ProcessAdd(AlgoStream<Bond>& data) override
    {
        bs_service->UpdateStream(data);
        auto price_stream = data.GetPriceStream();
        bs_service->PublishPrice(price_stream);
    }

    // no implementation
    void ProcessRemove(AlgoStream<Bond>& data) override {}

    // no implementation
    void ProcessUpdate(AlgoStream<Bond>& data) override {}

private:
    BondStreamingService* bs_service;
};

// Implement BondStreamingService class
void BondStreamingService::PublishPrice(PriceStream<Bond>& price_stream) {
    conn->Publish(price_stream);
}


void BondStreamingService::UpdateStream(const AlgoStream<Bond>& algo) {
    auto stream = algo.GetPriceStream();
    auto id = stream.GetProduct().GetProductId();
    if (streamMap.find(id) != streamMap.end()) { streamMap.erase(id); }

    streamMap.insert(pair<string, PriceStream<Bond> >(id, stream));

    for (auto& listener : listeners) {
        listener->ProcessAdd(stream);
    }
}


// Implement BondStreamingServiceConnector class
void BondStreamingServiceConnector::Publish(PriceStream<Bond>& data) {
    auto bond = data.GetProduct();
    auto bid = data.GetBidOrder();
    auto offer = data.GetOfferOrder();

    cout << "PriceSream -- product: " << bond.GetProductId() << "\n"
        << "\tBid\t" << "Price: " << bid.GetPrice() << "\tVisibleQuantity: " << bid.GetVisibleQuantity()
        << "\tHiddenQuantity: " << bid.GetHiddenQuantity() << "\n"
        << "\tAsk\t" << "Price: " << offer.GetPrice() << "\tVisibleQuantity: " << offer.GetVisibleQuantity()
        << "\tHiddenQuantity: " << offer.GetHiddenQuantity() << "\n";
}

#endif