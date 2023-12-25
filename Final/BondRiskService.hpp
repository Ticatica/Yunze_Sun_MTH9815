/**
* BondRiskService.hpp
* Definition of BondRiskService class
*
* 1 Listener
* BondRiskServiceListener - BondRiskService listen from BondPositionService
*
* @Yunze Sun
*/
#pragma once
#ifndef BondRiskService_h
#define BondRiskService_h

#include "BondPositionService.hpp"
#include "riskservice.hpp"
#include "utility.h"    // get pv01 value for each option
#include <iostream>

class BondRiskService : public RiskService<Bond> {
public:
    // ctor
    BondRiskService() { 
        riskMap = map<string, PV01<Bond> >();
        listeners = vector<ServiceListener<PV01<Bond>>*>();
    }

    // Implement the virtual func
    PV01<Bond>& GetData(string key) override { return riskMap.at(key); }

    // No need to implement because there's no linked connector
    void OnMessage(PV01<Bond>& data) override {}

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<PV01<Bond> >* listener) override {
        listeners.push_back(listener);
    }

    // Get all listeners on the Service.
    const vector<ServiceListener<PV01<Bond> >*>& GetListeners() const override {
        return listeners;
    }

    // called by BondPositionServiceListener
    void AddPosition(Position<Bond>& position);

    // Get the bucketed risk for the bucket sector
    PV01< BucketedSector<Bond> >& GetBucketedRisk(const BucketedSector<Bond>& sector) const;


private:
    map<string, PV01<Bond> > riskMap;
    vector<ServiceListener<PV01<Bond> >*> listeners;
};



class BondRiskServiceListener : public ServiceListener<Position<Bond> > {
private:
    BondRiskService* br_service;

public:
    // ctor
    BondRiskServiceListener(BondRiskService* _br_service) : br_service(_br_service) {}

    // override all pure virtual functions in base class
    // call bondPositionService.AddTrade
    void ProcessAdd(Position<Bond>& data) override { br_service->AddPosition(data); };

    // no implementation
    void ProcessRemove(Position<Bond>& data) override {}

    // no implementation
    void ProcessUpdate(Position<Bond>& data) override {}

};



void BondRiskService::AddPosition(Position<Bond>& position) {
    auto bond = position.GetProduct();
    string id = bond.GetProductId();
    double _pv01 = GetPV01Value(id);
    long _quantity = position.GetAggregatePosition();

    if (riskMap.find(id) != riskMap.end()) { riskMap.erase(id); }
    riskMap.insert(pair<string, PV01<Bond>>( id, PV01<Bond>(bond, _pv01, _quantity)));
    for (auto& listener : listeners) {
        listener->ProcessAdd(riskMap.find(id)->second);
    }
}


PV01< BucketedSector<Bond> >& BondRiskService::GetBucketedRisk(const BucketedSector<Bond>& _sector) const {

    double tot = 0;
    long _quantity = 0;

    auto _products = _sector.GetProducts();
    for (auto& p : _products)
    {
        string id = p.GetProductId();
        if (riskMap.find(id) != riskMap.end()) {
            tot += riskMap.at(id).GetPV01() * riskMap.at(id).GetQuantity();
            _quantity += riskMap.at(id).GetQuantity();
        }
    }

    PV01<BucketedSector<Bond>> pv01(_sector, tot, _quantity);
    return pv01;
}

#endif