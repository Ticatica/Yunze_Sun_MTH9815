/**
* ProductService.hpp
* Accessible all services and connectors
* Store all trading products with its ids
* 
* @Yunze Sun
*/

#ifndef productservice_hpp
#define productservice_hpp

#include "soa.hpp"
#include <iostream>
#include <map>

template <typename T>
class ProductService : public Service<string, T>
{

public:
    // ctor
    ProductService() { products = map<string, T>(); };
    ProductService(const vector<T> &_products){
        for (const auto& product : _products)
            products.insert(pair<string, T>(product.GetProductId(), product));
    };

    // Implement all the virtual functions
    T& GetData(string key) override { return products.at(key); };

    // no implementation; it's public
    void OnMessage(T & data) override {}
    void AddListener(ServiceListener<T>* listener) override {}
    const vector< ServiceListener<T>* >& GetListeners() const override { return vector< ServiceListener<T>* >(); };

private:
    map<string, T> products;

};

#endif