//
//  main.cpp
// 
//  MTH9815_Final_TradingSystem
//
//  @ Yunze_Sun
//
# pragma once

#include <iostream>
#include <vector>
#include "products.hpp"
#include "DataGenerator.hpp"

#include <string>
#include <iomanip>

#include "utility.h"
#include "ProductService.hpp"

#include "BondMarketDataService.hpp"
#include "BondPricingService.hpp"
#include "BondTradeBookingService.hpp"
#include "BondInquiryService.hpp"
#include "BondAlgoExecutionService.hpp"
#include "BondAlgoStreamingService.hpp"
#include "BondExecutionService.hpp"
#include "BondStreamingService.hpp"
#include "BondPositionService.hpp"
#include "BondRiskService.hpp"
#include "GUIService.hpp"
#include "BondHistoricalDataService.hpp"

using namespace std;

int main() {
    
    cout << "Generating predicting prices and orderbooks..." << endl;
    
    vector<string> bondCusip = { "9128283H1", "9128283L2", "912828M80", "9128283J7", "9128283F5", "912810TM0", "912810RZ3" };
    genOrderBook(bondCusip);
    cout << "Generating trades..." << endl;
    genTrades(bondCusip);
    cout << "Generating inquiries..." << endl;
    genInquiries(bondCusip);


    vector<Bond> bonds;
    for (int i = 0; i < bondCusip.size(); i++) {
        bonds.push_back(GetBond(bondCusip[i]));
    }
    cout << "finished";
    ProductService<Bond>* bondproductservice = new ProductService<Bond>(bonds);
    
    BondPricingService* bondpricingservice = new BondPricingService();
    BondPricingServiceConnector* bondpricingserviceconnector = new BondPricingServiceConnector(bondpricingservice, bondproductservice);

    BondAlgoStreamingService* bondalgostreamingservice = new BondAlgoStreamingService();
    BondAlgoStreamingServiceListener* bondalgostreamingservicelistener = new BondAlgoStreamingServiceListener(bondalgostreamingservice);

    bondpricingservice->AddListener(bondalgostreamingservicelistener);

    BondStreamingServiceConnector* bondstreamingserviceconnector = new BondStreamingServiceConnector();
    BondStreamingService* bondstreamingservice = new BondStreamingService(bondstreamingserviceconnector);
    BondStreamingServiceListener* bondstreamingservicelistener = new BondStreamingServiceListener(bondstreamingservice);

    bondalgostreamingservice->AddListener(bondstreamingservicelistener);




    BondMarketDataService* bondmarketdataservice = new BondMarketDataService();
    BondMarketDataServiceConnector* bondmarketdataserviceconnector = new BondMarketDataServiceConnector(bondmarketdataservice, bondproductservice);

    BondAlgoExecutionService* bondalgoexecutionservice = new BondAlgoExecutionService();
    BondAlgoExecutionServiceListener* bondalgoexecutionservicelistener = new BondAlgoExecutionServiceListener(bondalgoexecutionservice);

    bondmarketdataservice->AddListener(bondalgoexecutionservicelistener);

    BondExecutionServiceConnector* bondexecutionserviceconnector = new BondExecutionServiceConnector();
    BondExecutionService* bondexecutionservice = new BondExecutionService(bondexecutionserviceconnector);
    BondExecutionServiceListener* bondexecutionservicelistener = new BondExecutionServiceListener(bondexecutionservice);

    bondalgoexecutionservice->AddListener(bondexecutionservicelistener);




    BondTradeBookingService* bondtradebookingservice = new BondTradeBookingService();
    BondTradeBookingServiceConnector* bondtradebookingserviceconnector = new BondTradeBookingServiceConnector(bondtradebookingservice);
    BondTradeBookingServiceListener* bondtradebookingservicelistener = new BondTradeBookingServiceListener(bondtradebookingservice);

    bondexecutionservice->AddListener(bondtradebookingservicelistener);

    BondPositionService* bondpositionservice = new BondPositionService();
    BondPositionServiceListener* bondpositionservicelistener = new BondPositionServiceListener(bondpositionservice);

    bondtradebookingservice->AddListener(bondpositionservicelistener);

    //BondRiskService* bondriskservice = new BondRiskService();
    //BondRiskServiceListener* bondriskservicelistener = new BondRiskServiceListener(bondriskservice);

    //bondpositionservice->AddListener(bondriskservicelistener);


    BondInquiryServiceConnector2* bis_conn2 = new BondInquiryServiceConnector2();
    BondInquiryService* bondinquiryservice = new BondInquiryService(bis_conn2);
    BondInquiryServiceConnector* bondinquiryserviceconnector = new BondInquiryServiceConnector(bondinquiryservice, bondproductservice);

    GUIService<Bond>* guiservice = new GUIService<Bond>();
    GUIConnector<Bond>* guiserviceconnector = new GUIConnector<Bond>(guiservice);
    GUIServiceListener<Bond>* guiservicelistener = new GUIServiceListener<Bond>(guiservice);

    bondpricingservice->AddListener(guiservicelistener);




    BondHistoricalPositionServiceConnector* bondhistoricalpositionserviceconnector = new BondHistoricalPositionServiceConnector();
    BondHistoricalPositionService* bondhistoricalpositionservice = new BondHistoricalPositionService(bondhistoricalpositionserviceconnector);
    BondHistoricalPositionServiceListener* bondhistoricalpositionservicelistener = new BondHistoricalPositionServiceListener(bondhistoricalpositionservice);
    bondpositionservice->AddListener(bondhistoricalpositionservicelistener);

    BondHistoricalRiskServiceConnector* bondhistoricalriskserviceconnector = new BondHistoricalRiskServiceConnector();
    BondHistoricalRiskService* bondhistoricalriskservice = new BondHistoricalRiskService(bondhistoricalriskserviceconnector);
    BondHistoricalRiskServiceListener* bondhistoricalriskservicelistener = new BondHistoricalRiskServiceListener(bondhistoricalriskservice);
    //bondriskservice->AddListener(bondhistoricalriskservicelistener);

    BondHistoricalExecutionServiceConnector* bondhistoricalexecutionserviceconnector = new BondHistoricalExecutionServiceConnector();
    BondHistoricalExecutionService* bondhistoricalexecutionservice = new BondHistoricalExecutionService(bondhistoricalexecutionserviceconnector);
    BondHistoricalExecutionServiceListener* bondhistoricalexecutionservicelistener = new BondHistoricalExecutionServiceListener(bondhistoricalexecutionservice);
    bondexecutionservice->AddListener(bondhistoricalexecutionservicelistener);

    BondHistoricalStreamingServiceConnector* bondhistoricalstreamingserviceconnector = new BondHistoricalStreamingServiceConnector();
    BondHistoricalStreamingService* bondhistoricalstreamingservice = new BondHistoricalStreamingService(bondhistoricalstreamingserviceconnector);
    BondHistoricalStreamingServiceListener* bondhistoricalstreamingservicelistener = new BondHistoricalStreamingServiceListener(bondhistoricalstreamingservice);
    bondstreamingservice->AddListener(bondhistoricalstreamingservicelistener);

    BondHistoricalInquiryServiceConnector* bondhistoricalinquiryserviceconnector = new BondHistoricalInquiryServiceConnector();
    BondHistoricalInquiryService* bondhistoricalinquiryservice = new BondHistoricalInquiryService(bondhistoricalinquiryserviceconnector);
    BondHistoricalInquiryServiceListener* bondhistoricalinquiryservicelistener = new BondHistoricalInquiryServiceListener(bondhistoricalinquiryservice);
    bondinquiryservice->AddListener(bondhistoricalinquiryservicelistener);


    bondpricingserviceconnector->Subscribe();
    bondmarketdataserviceconnector->Subscribe();
    bondtradebookingserviceconnector->Subscribe();
    bondinquiryserviceconnector->Subscribe();
    
    return 0;
    
}