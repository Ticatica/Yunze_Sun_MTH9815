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

#ifndef BondExecutionService_h
#define BondExecutionService_h

#include "BondAlgoExecutionService.hpp"
#include "soa.hpp"
using namespace std;

class BondExecutionServiceConnector;

class BondExecutionService : public ExecutionService<Bond> {
private:
    map<string, ExecutionOrder<Bond> > exeMap;
    vector<ServiceListener<ExecutionOrder<Bond> >*> listeners;
    BondExecutionServiceConnector* conn; // connector to publish executions

public:
    // ctor
    BondExecutionService(BondExecutionServiceConnector* _conn) : conn(_conn) { exeMap = map<string, ExecutionOrder<Bond> >(); };

    // Implement all the virtual functions

    ExecutionOrder<Bond>& GetData(string key) override { return exeMap.at(key); }

    // no need for implementation here
    void OnMessage(ExecutionOrder<Bond>& data) override {}

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<ExecutionOrder<Bond> >* listener) override { listeners.push_back(listener); }

    // Get all listeners on the Service.
    const vector<ServiceListener<ExecutionOrder<Bond> >*>& GetListeners() const override { return listeners; }

    // add price based on orderbook, called by listener when adding process
    void ExecuteOrder(ExecutionOrder<Bond>& exe_order, Market market);

    // add execution from algo and notify listeners
    void AddExecution(const AlgoExecution<Bond>& algo);
};



class BondExecutionServiceConnector : public Connector<ExecutionOrder<Bond> > {
public:
    // ctor
    BondExecutionServiceConnector() {}

    // publish the order
    void Publish(ExecutionOrder<Bond>& order, Market market);
    void Publish(ExecutionOrder<Bond>& order) override { Publish(order, CME); };

    // publish-only, no implementaion
    void Subscribe() {}
};


class BondExecutionServiceListener : public ServiceListener<AlgoExecution<Bond> > {
public:
    // ctor
    BondExecutionServiceListener(BondExecutionService* _be_service) : be_service(_be_service) {};

    // override virtual functions in ServiceListener class
    // listen from BondAlgoExecutionService
    // and call BondExecutionService::AddAlgoExecution and BondExecutionService::ExecuteOrder
    void ProcessAdd(AlgoExecution<Bond>& data) override
    {
        auto exe_order = data.GetOrder();
        be_service->AddExecution(data);
        be_service->ExecuteOrder(exe_order, data.GetMarket());
    }

    // no implementation
    void ProcessRemove(AlgoExecution<Bond>& data) override {}

    // no implementation
    void ProcessUpdate(AlgoExecution<Bond>& data) override {}

private:
    BondExecutionService* be_service;
};


// Implement BondExecutionService class
void BondExecutionService::ExecuteOrder(ExecutionOrder<Bond>& exe_order, Market market) {
    conn->Publish(exe_order, market);
}


void BondExecutionService::AddExecution(const AlgoExecution<Bond>& algo_exe) {
    auto exe_order = algo_exe.GetOrder();
    string order_id = exe_order.GetProduct().GetProductId();

    // update executionMap
    if (exeMap.find(order_id) != exeMap.end()) { exeMap.erase(order_id); }
    exeMap.insert(pair<string, ExecutionOrder<Bond> >(order_id, exe_order));

    for (auto& listener : listeners) {
        listener->ProcessAdd(exe_order);
    }
}


// Implement BondExecutionServiceConnecto class
void BondExecutionServiceConnector::Publish(ExecutionOrder<Bond>& data, Market market) 
{
    auto bond = data.GetProduct();
    string oder_type;
    switch (data.GetOrderType()) {
    case FOK: oder_type = "FOK"; break;
    case MARKET: oder_type = "MARKET"; break;
    case LIMIT: oder_type = "LIMIT"; break;
    case STOP: oder_type = "STOP"; break;
    case IOC: oder_type = "IOC"; break;
    }
    cout << bond.GetProductId() << " OrderId: " << data.GetOrderId() << "\n"
        << " OrderType: " << oder_type
        << "\nPrice: " << data.GetPrice() << "\tVisibleQuantity: " << data.GetVisibleQuantity()
        << "\tHiddenQuantity: " << data.GetHiddenQuantity() << endl << endl;
}

#endif