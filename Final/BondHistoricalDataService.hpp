/**
* BondExecutionService.hpp
* Definition of BondExecutionService class
*
* 1 Connector: BondExecutionServiceConnector
* Publish the execution order
*
*
* 1 Listener: BondExecutionServiceListener
* listen from BondAlgoExecutionService
*
* @Yunze Sun
*/

#ifndef BondHistoricalDataService_h
#define BondHistoricalDataService_h

#include <iostream>
#include "BondPositionService.hpp"
#include "BondRiskService.hpp"
#include "BondExecutionService.hpp"
#include "BondStreamingService.hpp"
#include "BondInquiryService.hpp"
#include "historicaldataservice.hpp"
#include "soa.hpp"
#include "products.hpp"


class BondHistoricalPositionServiceConnector;
class BondHistoricalRiskServiceConnector;
class BondHistoricalExecutionServiceConnector;
class BondHistoricalStreamingServiceConnector;
class BondHistoricalInquiryServiceConnector;

class BondHistoricalPositionService : public HistoricalDataService<Position<Bond> > {
public:
    // ctor
    BondHistoricalPositionService(BondHistoricalPositionServiceConnector* _connector) :connector(_connector) {}

    //override all functions
    Position<Bond>& GetData(string key) override { return dataMap.at(key); }
    void OnMessage(Position<Bond>& data) override {}
    void AddListener(ServiceListener<Position<Bond>  >* listener) override {}
    const vector<ServiceListener<Position<Bond>  >*>& GetListeners() const override { return listeners; }
    void PersistData(string persistKey, const Position<Bond>& data) override;

private:
    map<string, Position<Bond> > dataMap;
    BondHistoricalPositionServiceConnector* connector;
    vector<ServiceListener<Position<Bond> >*> listeners;
};

class BondHistoricalRiskService : public HistoricalDataService<PV01<Bond> > {
private:
    map<string, PV01<Bond> > dataMap;
    BondHistoricalRiskServiceConnector* connector;
    vector<ServiceListener<PV01<Bond> >*> listeners;
public:
    // ctor
    BondHistoricalRiskService(BondHistoricalRiskServiceConnector* _connector) :connector(_connector) {}

    //override all functions
    PV01<Bond>& GetData(string id) override { return dataMap.at(id); }
    void OnMessage(PV01<Bond>& data) override {}
    void AddListener(ServiceListener<PV01<Bond>  >* listener) override {}
    const vector<ServiceListener<PV01<Bond>  >*>& GetListeners() const override { return listeners; }
    void PersistData(string persistKey, const PV01<Bond>& data) override;
};

class BondHistoricalExecutionService : public HistoricalDataService<ExecutionOrder<Bond> > {

public:
    // ctor
    BondHistoricalExecutionService(BondHistoricalExecutionServiceConnector* _connector) :connector(_connector) {}

    //override all functions
    ExecutionOrder<Bond>& GetData(string id) override { return dataMap.at(id); }
    void OnMessage(ExecutionOrder<Bond>& data) override {}
    void AddListener(ServiceListener<ExecutionOrder<Bond>  >* listener) override {}
    const vector<ServiceListener<ExecutionOrder<Bond>  >*>& GetListeners() const override { return listeners; }
    void PersistData(string persistKey, const ExecutionOrder<Bond>& data) override;

private:
    map<string, ExecutionOrder<Bond> > dataMap;
    BondHistoricalExecutionServiceConnector* connector;
    vector<ServiceListener<ExecutionOrder<Bond> >*> listeners;
};

class BondHistoricalStreamingService : public HistoricalDataService<PriceStream<Bond> > {

public:
    // ctor
    BondHistoricalStreamingService(BondHistoricalStreamingServiceConnector* _connector) :connector(_connector) {}

    //override all functions
    PriceStream<Bond>& GetData(string id) override { return dataMap.at(id); }
    void OnMessage(PriceStream<Bond>& data) override {}
    void AddListener(ServiceListener<PriceStream<Bond>  >* listener) override {}
    const vector<ServiceListener<PriceStream<Bond>  >*>& GetListeners() const override { return listeners; }
    void PersistData(string persistKey, const PriceStream<Bond>& data) override;

private:
    map<string, PriceStream<Bond> > dataMap;
    BondHistoricalStreamingServiceConnector* connector;
    vector<ServiceListener<PriceStream<Bond> >*> listeners;
};

class BondHistoricalInquiryService : public HistoricalDataService<Inquiry<Bond> > {

public:
    // ctor
    BondHistoricalInquiryService(BondHistoricalInquiryServiceConnector* _connector) :connector(_connector) {}

    //override all functions
    Inquiry<Bond>& GetData(string id) override { return dataMap.at(id); }
    void OnMessage(Inquiry<Bond>& data) override {}
    void AddListener(ServiceListener<Inquiry<Bond>  >* listener) override {}
    const vector<ServiceListener<Inquiry<Bond>  >*>& GetListeners() const override { return listeners; }
    void PersistData(string persistKey, const Inquiry<Bond>& data) override;

private:
    map<string, Inquiry<Bond> > dataMap;
    BondHistoricalInquiryServiceConnector* connector;
    vector<ServiceListener<Inquiry<Bond> >*> listeners;
};

// Connector Service

class BondHistoricalPositionServiceConnector : public Connector<Position<Bond> > {
public:
    // ctor
    BondHistoricalPositionServiceConnector() {};

    //override all functions
    void Subscribe() {}
    void Publish(Position<Bond>& data) override {
        auto id = data.GetProduct().GetProductId();
        auto data_needed = data.GetAggregatePosition();

        ofstream out;
        out.open("positions.txt", ios::app);
        out << id << "," << data_needed << endl;
    }
};

void BondHistoricalPositionService::PersistData(string persistKey, const Position<Bond>& data) {
    auto id = data.GetProduct().GetProductId();
    if (dataMap.find(id) != dataMap.end())
        dataMap.erase(id);
    dataMap.insert(pair<string, Position<Bond> >(id, data));
    auto data_temp = data;
    connector->Publish(data_temp);
    return;
}



class BondHistoricalRiskServiceConnector : public Connector<PV01<Bond> > {
public:
    // ctor
    BondHistoricalRiskServiceConnector() {};

    //override all functions
    void Subscribe() {}
    void Publish(PV01<Bond>& data) override {
        auto id = data.GetProduct().GetProductId();
        auto data_needed = data.GetPV01();

        ofstream out;
        out.open("risk.txt", ios::app);
        out << id << "," << data_needed << endl;
    }
};

void BondHistoricalRiskService::PersistData(string persistKey, const PV01<Bond>& data) {
    auto bond = data.GetProduct();
    auto id = bond.GetProductId();
    if (dataMap.find(id) != dataMap.end())
        dataMap.erase(id);
    dataMap.insert(pair<string, PV01<Bond> >(id, data));
    auto data_temp = data;
    connector->Publish(data_temp);
}



class BondHistoricalExecutionServiceConnector : public Connector<ExecutionOrder<Bond> > {
public:
    // ctor
    BondHistoricalExecutionServiceConnector() {};

    //override all functions
    void Subscribe() {}
    void Publish(ExecutionOrder<Bond>& data) override {
        auto id = data.GetProduct().GetProductId();
        string oderType;
        switch (data.GetOrderType()) {
        case FOK: oderType = "FOK"; break;
        case MARKET: oderType = "MARKET"; break;
        case LIMIT: oderType = "LIMIT"; break;
        case STOP: oderType = "STOP"; break;
        case IOC: oderType = "IOC"; break;
        }

        ofstream out;
        out.open("executions.txt", ios::app);
        out << id << "," << data.GetOrderId()
            << "," << (data.GetSide() == BID ? "Bid" : "Ask")
            << "," << oderType << "," << (data.IsChildOrder() ? "True" : "False")
            << "," << data.GetPrice()
            << "," << data.GetVisibleQuantity()
            << "," << data.GetHiddenQuantity() << endl;
    }
};

void BondHistoricalExecutionService::PersistData(string persistKey, const ExecutionOrder<Bond>& data) {
    auto id = data.GetOrderId();
    if (dataMap.find(id) != dataMap.end())
        dataMap.erase(id);
    dataMap.insert(pair<string, ExecutionOrder<Bond> >(id, data));
    auto data_temp = data;
    connector->Publish(data_temp);
}




class BondHistoricalStreamingServiceConnector : public Connector<PriceStream<Bond> > {
public:
    // ctor
    BondHistoricalStreamingServiceConnector() {};

    //override all functions
    void Subscribe() {}
    void Publish(PriceStream<Bond>& data) override {
        auto id = data.GetProduct().GetProductId();
        auto bid = data.GetBidOrder();
        auto offer = data.GetOfferOrder();

        ofstream out;
        out.open("streaming.txt", ios::app);

        out << id << ","
            << bid.GetPrice() << "," << bid.GetVisibleQuantity()
            << "," << bid.GetHiddenQuantity()
            << "," << offer.GetPrice() << "," << offer.GetVisibleQuantity()
            << "," << offer.GetHiddenQuantity()
            << endl;
    }
};

void BondHistoricalStreamingService::PersistData(string persistKey, const PriceStream<Bond>& data) {
    auto id = data.GetProduct().GetProductId();
    if (dataMap.find(id) != dataMap.end())
        dataMap.erase(id);
    dataMap.insert(pair<string, PriceStream<Bond> >(id, data));
    auto data_temp = data;
    connector->Publish(data_temp);
}




class BondHistoricalInquiryServiceConnector : public Connector<Inquiry<Bond> > {
public:
    // ctor
    BondHistoricalInquiryServiceConnector() {};

    //override all functions
    void Subscribe() {}
    void Publish(Inquiry<Bond>& data) override {
        auto id = data.GetProduct().GetProductId();
        auto price = data.GetPrice();
        auto quantity = data.GetQuantity();
        string side = (data.GetSide() == BUY ? "BUY" : "SELL");
        auto inquiry_id = data.GetInquiryId();
        string state_;
        switch (data.GetState()) {
        case RECEIVED:state_ = "RECEIVED"; break;
        case QUOTED:state_ = "QUOTED"; break;
        case DONE:state_ = "DONE"; break;
        case REJECTED:state_ = "REJECTED"; break;
        case CUSTOMER_REJECTED:state_ = "CUSTOMER_REJECTED"; break;
        }

        ofstream out;
        out.open("inquiries.txt", ios::app);
        out << inquiry_id << "," << id << "," << price << "," << quantity << "," << side << "," << state_
            << endl;
    }
};

void BondHistoricalInquiryService::PersistData(string persistKey, const Inquiry<Bond>& data) {
    auto id = data.GetInquiryId();
    if (dataMap.find(id) != dataMap.end())
        dataMap.erase(id);
    dataMap.insert(pair<string, Inquiry<Bond> >(id, data));
    auto data_temp = data;
    connector->Publish(data_temp);
}


// Listner Service

class BondHistoricalPositionServiceListener : public ServiceListener<Position<Bond> > {
private:
    BondHistoricalPositionService* ser;

public:
    // ctor
    BondHistoricalPositionServiceListener(BondHistoricalPositionService* _s) :ser(_s) {}

    //override all functions
    void ProcessAdd(Position<Bond>& data) override {
        ser->PersistData(data.GetProduct().GetProductId(), data);
    }
    void ProcessRemove(Position<Bond>& data) override {}
    void ProcessUpdate(Position<Bond>& data) override {}
};

class BondHistoricalRiskServiceListener : public ServiceListener<PV01<Bond> > {
private:
    BondHistoricalRiskService* ser;

public:
    // ctor
    BondHistoricalRiskServiceListener(BondHistoricalRiskService* _s) :ser(_s) {}

    //override all functions
    void ProcessAdd(PV01<Bond>& data) override {
        ser->PersistData(data.GetProduct().GetProductId(), data);
    }
    void ProcessRemove(PV01<Bond>& data) override {}
    void ProcessUpdate(PV01<Bond>& data) override {}
};

class BondHistoricalExecutionServiceListener : public ServiceListener<ExecutionOrder<Bond> > {
private:
    BondHistoricalExecutionService* ser;

public:
    // ctor
    BondHistoricalExecutionServiceListener(BondHistoricalExecutionService* _s) :ser(_s) {}

    //override all functions
    void ProcessAdd(ExecutionOrder<Bond>& data) override {
        ser->PersistData(data.GetProduct().GetProductId(), data);
    }
    void ProcessRemove(ExecutionOrder<Bond>& data) override {}
    void ProcessUpdate(ExecutionOrder<Bond>& data) override {}
};

class BondHistoricalStreamingServiceListener : public ServiceListener<PriceStream<Bond> > {
private:
    BondHistoricalStreamingService* ser;

public:
    // ctor
    BondHistoricalStreamingServiceListener(BondHistoricalStreamingService* _s) :ser(_s) {}

    //override all functions
    void ProcessAdd(PriceStream<Bond>& data) override {
        ser->PersistData(data.GetProduct().GetProductId(), data);
    }
    void ProcessRemove(PriceStream<Bond>& data) override {}
    void ProcessUpdate(PriceStream<Bond>& data) override {}
};

class BondHistoricalInquiryServiceListener : public ServiceListener<Inquiry<Bond> > {
private:
    BondHistoricalInquiryService* ser;

public:
    // ctor
    BondHistoricalInquiryServiceListener(BondHistoricalInquiryService* _s) :ser(_s) {}

    //override all functions
    void ProcessAdd(Inquiry<Bond>& data) override {
        ser->PersistData(data.GetProduct().GetProductId(), data);
    }
    void ProcessRemove(Inquiry<Bond>& data) override {}
    void ProcessUpdate(Inquiry<Bond>& data) override {}
};

#endif