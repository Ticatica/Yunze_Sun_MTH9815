/**
* BondMarketDataService.hpp
* Definition of BondMarketDataService class
*
* 1 Connector
* read prediction from marketdata.txt
*
* @Yunze Sun
*/

#ifndef BondMarketDataService_h
#define BondMarketDataService_h

#include <fstream>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include "soa.hpp"
#include "marketdataservice.hpp"
#include "ProductService.hpp"
#include "products.hpp"
#include "utility.h"

class BondMarketDataService : public MarketDataService<Bond> {
public:
    // ctor
    BondMarketDataService() { orderMap = map<string, OrderBook<Bond> >(); };

    // Implement all the virtual functions
    
    // Get the best bid/offer order
    const BidOffer& GetBestBidOffer(const string& productId) override;
    // Aggregate the order book
    const OrderBook<Bond>& AggregateDepth(const string& productId) override;

    // override virtual func Service
    // Get data on our service given a key
    OrderBook<Bond>& GetData(string key) override
    {
        if (orderMap.find(key) == orderMap.end()) {
            orderMap.insert(pair<string, OrderBook<Bond>>(key, OrderBook<Bond>(GetBond(key))));
        }
        return orderMap.at(key);

    }

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(OrderBook<Bond>& data) override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<OrderBook<Bond> >* listener) override {
        listeners.push_back(listener);
    }
    // Get all listeners on the Service.
    const vector<ServiceListener<OrderBook<Bond> >*>& GetListeners() const override { return listeners; };


private:
    map<string, OrderBook<Bond> > orderMap;
    vector<ServiceListener<OrderBook<Bond> >*> listeners;
};




class BondMarketDataServiceConnector : public Connector<OrderBook<Bond> > {
public:
    BondMarketDataServiceConnector(BondMarketDataService* _bmd_service, ProductService<Bond>* _product_service)
        : bmd_service(_bmd_service), product_service(_product_service) {};

    // no publish
    void Publish(OrderBook<Bond>& info) override {};
    // subscribe data from marketdata.txt
    void Subscribe();

private:
    BondMarketDataService* bmd_service;
    ProductService<Bond>* product_service;
};



// Implement the BondMarketDataService class

const BidOffer& BondMarketDataService::GetBestBidOffer(const string& _productId) {
    return std::move(orderMap.at(_productId).GetBestBidOffer());
}


const OrderBook<Bond>& BondMarketDataService::AggregateDepth(const string& _productId) {
    auto orderbook = orderMap.at(_productId);   // get the orderbook of a specified product
    vector<Order> offers = orderbook.GetOfferStack();
    vector<Order> bids = orderbook.GetBidStack();

    // aggregate quantity by the same price
    unordered_map<double, long> bid_depth, ask_depth;
    auto agg_depth = [](const vector<Order>& stack, unordered_map<double, long>& depth) {
        for (auto& elem : stack) {
            if (depth.find(elem.GetPrice()) != depth.end()) {
                depth[elem.GetPrice()] += elem.GetQuantity();
            }
            else {
                depth[elem.GetPrice()] = elem.GetQuantity();
            }
        }
    };

    // aggregate bid and ask orderbook
    agg_depth(bids, bid_depth);
    agg_depth(offers, ask_depth);
    /*
    // sort the prices
    vector<double> bid_prices, ask_prices;

    for (const auto& bid : bid_depth) bid_prices.push_back(bid.first);
    for (const auto& ask : ask_depth) ask_prices.push_back(ask.first);

    sort(bid_prices.rbegin(), bid_prices.rend());
    sort(ask_prices.begin(), ask_prices.end());

    // order matching
    size_t bid_index = 0, ask_index = 0;
    while (bid_index < bid_prices.size() && ask_index < ask_prices.size()) {
        double bid_price = bid_prices[bid_index];
        double ask_price = ask_prices[ask_index];

        if (bid_price >= ask_price) {
            // Match occurs
            long trade_quantity = min(bid_depth[bid_price], ask_depth[ask_price]);

            // Update depths
            bid_depth[bid_price] -= trade_quantity;
            if (bid_depth[bid_price] == 0) bid_index++;

            ask_depth[ask_price] -= trade_quantity;
            if (ask_depth[ask_price] == 0) ask_index++;
        }
        else {
            // No more matches possible
            break;
        }
    }
    */
    // generate new order stacks after aggregation
    vector<Order> agg_bid, agg_ask;
    for (auto& elem : bid_depth) {
        agg_bid.push_back(Order(elem.first, elem.second, BID));
    }

    for (auto& elem : ask_depth) {
        agg_ask.push_back(Order(elem.first, elem.second, OFFER));
    }

    return OrderBook<Bond>(orderbook.GetProduct(), agg_bid, agg_ask);
}



void BondMarketDataService::OnMessage(OrderBook<Bond>& data) {
    // flow data
    string id = data.GetProduct().GetProductId();
    // update the order book
    if (orderMap.find(id) != orderMap.end()) { orderMap.erase(id); }
    orderMap.insert(pair<string, OrderBook<Bond> >(id, data));

    // get best order for listeners : algoexecution
    auto best_order = GetBestBidOffer(id);
    vector<Order> bid, ask;
    bid.push_back(best_order.GetBidOrder());
    ask.push_back(best_order.GetOfferOrder());
    auto best_order_book = OrderBook<Bond>(data.GetProduct(), bid, ask);

    for (auto& listener : listeners) {
        listener->ProcessAdd(best_order_book);
    }
}

// Implement the BondMarketDataServiceConnector class

void BondMarketDataServiceConnector::Subscribe() {
    // read data from marketdata.txt
    ifstream file("marketdata.txt", ios::in);
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

            // string timestamp = dataVec[0];
            string id = dataVec[1];

            auto ob = bmd_service->GetData(id);
            vector<Order> bids = ob.GetBidStack();
            vector<Order> asks = ob.GetOfferStack();

            for (int k = 0; k < 5; k++) {
                // bidPrice = dataVec[4 * k + 2];
                // bidSize = dataVec[4 * k + 3];
                // askPrice = dataVec[4 * k + 4];
                // askSize = dataVec[4 * k + 5];
                
                bids.push_back(Order(Str2Price(dataVec[4 * k + 2]), stol(dataVec[4 * k + 3]), BID));
                asks.push_back(Order(Str2Price(dataVec[4 * k + 4]), stol(dataVec[4 * k + 5]), OFFER));
            }
            // aggregate the order book, get a copy
            auto aggOrderBook = bmd_service->AggregateDepth(id);
            // publish the order book to the service
            bmd_service->OnMessage(aggOrderBook);
        }
    }
}



#endif