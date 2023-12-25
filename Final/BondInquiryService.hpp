/**
* BondInquiryService.hpp
* Definition of BondInquiryService class
*
* 2 Connectors: 
* BondInquiryServiceConnector: subscribe from inquiries.txt
* BondInquiryServiceConnector2: publish the quote
*
*
* @Yunze Sun
*/

#ifndef BondInquiryService_h
#define BondInquiryService_h

#include "soa.hpp"
#include "inquiryservice.hpp"
#include "utility.h"
#include "productservice.hpp"
#include <fstream>
class BondInquiryServiceConnector2;

class BondInquiryService : public InquiryService<Bond> {
private:
    map<string, Inquiry<Bond> > inquiryMap;
    vector<ServiceListener<Inquiry<Bond> >* > listeners;
    BondInquiryServiceConnector2* conn;
public:
    // ctor
    BondInquiryService(BondInquiryServiceConnector2* _conn) : conn(_conn){ inquiryMap = map<string, Inquiry<Bond> >(); }

    // Implement all the virtual functions

    Inquiry<Bond>& GetData(string key) override { return inquiryMap.at(key); };

    // The callback that a Connector should invoke for any new or updated data
    // add new bond price to pricemap
    void OnMessage(Inquiry<Bond>& data) override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<Inquiry<Bond> >* listener) override { listeners.push_back(listener); }

    // Get all listeners on the Service.
    const vector<ServiceListener<Inquiry<Bond> >*>& GetListeners() const override { return listeners; }

    // Send a quote back to the client
    void SendQuote(const string& inquiryId, double price);

    // Reject an inquiry from the client
    void RejectInquiry(const string& inquiryId);

    // Set Connector
    void SetConn(BondInquiryServiceConnector2* _conn) { conn = _conn; }
};

// connector 1 - subscribe from inquiries.txt
class BondInquiryServiceConnector : public Connector<Inquiry<Bond> > {
public:
    // ctor
    BondInquiryServiceConnector(BondInquiryService* _bi_service, ProductService<Bond>* _products)
        : bi_service(_bi_service), products(_products) {};

    // no need for implementation
    void Publish(Inquiry<Bond>& info) override {};
    // subscribe data from inquiry.txt file
    void Subscribe();

private:
    BondInquiryService* bi_service;
    ProductService<Bond>* products;

};

// connector 2 - publish the quote
class BondInquiryServiceConnector2 : public Connector<Inquiry<Bond> > {
public:
    // ctor
    BondInquiryServiceConnector2() {};

    // publish the quote and transit RECEIVED to QUOTED
    void Publish(Inquiry<Bond>& data) override
    {
        if (data.GetState() == RECEIVED) {
            data.SetState(QUOTED);
        }
    }
    // publish-only
    void Subscribe() {};

};

// Implement BondInquiryService class
void BondInquiryService::OnMessage(Inquiry<Bond>& data) {
    InquiryState state = data.GetState();
    string inquiryId = data.GetInquiryId();
    switch (state) {
    case RECEIVED:
        // if inquiry is received, send back a quote to the connector via publish()
        conn->Publish(data);
        break;
    case QUOTED:
        // finish the inquiry with DONE status and send an update of the object
        data.SetState(DONE);
        // store the inquiry
        if (inquiryMap.find(inquiryId) != inquiryMap.end()) { inquiryMap.erase(inquiryId); }
        inquiryMap.insert(pair<string, Inquiry<Bond> >(inquiryId, data));
        // notify listeners
        for (auto& listener : listeners)
        {
            listener->ProcessAdd(data);
        }
        break;
    default:
        break;
    }


    // if done, remove the inquiry from the map
    if ((data.GetState() == DONE) or (inquiryMap.find(inquiryId) != inquiryMap.end()))
    {
        inquiryMap.erase(inquiryId);
    }
    // otherwise, update the inquiry
    else
    {
        inquiryMap.insert(pair<string, Inquiry<Bond>>(inquiryId, data));
    }

    // notify listeners
    for (auto& listener : listeners)
    {
        listener->ProcessAdd(data);
    }
}

void BondInquiryService::SendQuote(const string& inquiryId, double price) {
    // get the inquiry
    Inquiry<Bond>& inquiry = GetData(inquiryId);
    // update the inquiry
    inquiry.SetState(inquiry.GetState(), price);
    // notify listeners
    for (auto& listener : listeners)
    {
        listener->ProcessAdd(inquiry);
    }
}


void BondInquiryService::RejectInquiry(const string& inquiryId) {
    // get the inquiry
    Inquiry<Bond>& inquiry = GetData(inquiryId);
    // update the inquiry
    inquiry.SetState(REJECTED);
}


void BondInquiryServiceConnector::Subscribe() {
    // read data from file
    ifstream file("inquiries.txt", ios::in);
    if (file.is_open()) {
        string _line, _data;
        while (getline(file, _line)) {
            stringstream line(_line);
            vector<string> dataVec;
            while (getline(line, _data, ','))
            {
                dataVec.push_back(_data);
            }
            string inquiryId = dataVec[0];
            string productId = dataVec[1];
            Bond bond = products->GetData(productId);
            Side side = dataVec[2] == "BUY" ? BUY : SELL;
            long quantity = stol(dataVec[3]);
            double price = Str2Price(dataVec[4]);
            InquiryState state = dataVec[5] == "RECEIVED" ? RECEIVED : dataVec[5] == "QUOTED" ? QUOTED : dataVec[5] == "DONE" ? DONE : dataVec[5] == "REJECTED" ? REJECTED : CUSTOMER_REJECTED;
            Inquiry<Bond> inquiry(inquiryId, bond, side, quantity, price, state);
            bi_service->OnMessage(inquiry);
        }
    }
}

# endif