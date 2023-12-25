/**
* BondPricingService.hpp
* Definition of BondPricingService class
*
* 1 Connector
* read prediction from price.txt
*
* @Yunze Sun
*/

#ifndef BondPricingService_h
#define BondPricingService_h

#include "fstream"
#include "pricingservice.hpp"
#include "products.hpp"
#include "soa.hpp"
#include "utility.h"

using namespace std;

class BondPricingService : public PricingService<Bond> {
private:
    map<string, Price<Bond> > priceMap;
    vector<ServiceListener<Price<Bond> >* > listeners;

public:
    // ctor
    BondPricingService() { priceMap = map<string, Price<Bond> >(); };

    // Implement all the virtual functions

    Price<Bond>& GetData(string key) override { return priceMap.at(key); };

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Price<Bond>& data) override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<Price<Bond> >* listener) override {
        listeners.push_back(listener);
    }

    // Get all listeners on the Service.
    const vector<ServiceListener<Price<Bond> >*>& GetListeners() const override {
        return listeners;
    };
};



class BondPricingServiceConnector : public Connector<Price<Bond> > {
public:
    BondPricingServiceConnector(BondPricingService* _bp_service, ProductService<Bond>* _product_service) 
        : bp_service(_bp_service), product_service(_product_service){};

    // subscribe-only where Publish() does nothing
    void Publish(Price<Bond>& info) override {};
    // subscribe data from price.txt file
    void Subscribe();

private:
    BondPricingService* bp_service;
    ProductService<Bond>* product_service;
};


void BondPricingService::OnMessage(Price<Bond>& data) {

    string id = data.GetProduct().GetProductId();

    if (priceMap.find(id) != priceMap.end()) { priceMap.erase(id); }
    priceMap.insert(pair<string, Price<Bond> >(id, data));

    // flow the data to listeners
    for (auto& listener : listeners) {
        listener->ProcessAdd(data);
    }
}


void BondPricingServiceConnector::Subscribe() {
    // read data from price.txt
    ifstream file("price.txt", ios::in);
    if (file.is_open()) {
        string _line, _data;
        getline(file, _line);
        while (getline(file, _line)) {
            stringstream line(_line);
            vector<string> dataVec;
            while (getline(line, _data, ','))
            {
                dataVec.push_back(_data);
            }

            string _productId = dataVec[1];
            auto _product = product_service->GetData(_productId);

            double _bidPrice = Str2Price(dataVec[2]);
            double _offerPrice = Str2Price(dataVec[3]);
            double _midPrice = (_bidPrice + _offerPrice) / 2.0;
            double _spread = _offerPrice - _bidPrice;
            
            
            Price<Bond> _price(_product, _midPrice, _spread);

            // flow the data
            bp_service->OnMessage(_price);
        }
    }
}

#endif