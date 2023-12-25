/**
* DataGenerator.hpp
* Generate data cases
*
* 1 Connector
* BondTradeBookingServiceConnector - Read data from "trades.txt"
*
* 1 Listener
* BondTradeBookingServiceListener - BondTradeBookingService listen from BondExecutionService
*
* @Yunze Sun
*/
#ifndef DATA_GENERATOR_HPP
#define DATA_GENERATOR_HPP

#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <fstream>
#include <random>
#include "utility.h"

using namespace std;

// Generate random ID with numbers and letters
string GenerateRandomId(long length)
{
    string id = "";
    for (int j = 0; j < length; ++j) {
        int random = rand() % 36;
        if (random < 10) {
            id += to_string(random);
        }
        else {
            id += static_cast<char>('A' + random - 10);
        }
    }
    return id;
}

// generate the spread
// The bid/offer spread should oscillate between 1/128 and 1/64.
double genRandomSpread(std::mt19937& gen) {
    std::uniform_real_distribution<double> dist(1.0 / 128.0, 1.0 / 64.0);
    return dist(gen);
}

/**
* 1. Generate the predicting prices (recorded in price.txt)
* 2. Generate the order books (recorded in marketdata.txt)
*/
void genOrderBook(const vector<string>& products, const string& _priceFileName="price.txt", 
    const string& _bookFileName="marketdata.txt", long long seed=12345, const long numStep= 10'000)
{
    std::ofstream os_price(_priceFileName);
    std::ofstream os_book(_bookFileName);
    std::mt19937 gen(seed);
    long long t = 0;

    // price file format: Timestamp, CUSIP, Bid, Ask
    os_price << "Timestamp,CUSIP,Bid,Ask" << endl;

    // orderbook file format: Timestamp, CUSIP, Bid1, BidSize1, Ask1, AskSize1, Bid2, BidSize2, Ask2, AskSize2, Bid3, BidSize3, Ask3, AskSize3, Bid4, BidSize4, Ask4, AskSize4, Bid5, BidSize5, Ask5, AskSize5
    os_book << "Timestamp,CUSIP,Bid1,BidSize1,Ask1,AskSize1,Bid2,BidSize2,Ask2,AskSize2,Bid3,BidSize3,Ask3,AskSize3,Bid4,BidSize4,Ask4,AskSize4,Bid5,BidSize5,Ask5,AskSize5" << endl;

    double midPrice = 99.00;
    bool priceIncreasing = true;
    bool spreadIncreasing = true;
    double tick_size = 1.0 / 256.0;
    double book_spread = 1.0 / 128.0;

    for (long i = 0; i < numStep; ++i) {

        for (const auto& product : products)
        {
            // generate price data
            double spread = genRandomSpread(gen);
            double myBidPrice = midPrice - spread / 2.0;
            double myOfferPrice = midPrice + spread / 2.0;

            os_price << setw(12) << std::setfill('0') << t << "," << product << ","
                << Price2Str(myBidPrice) << "," << Price2Str(myOfferPrice) << endl;

            // generate order book data
            os_book << setw(12) << std::setfill('0') << t << "," << product;
            double bidPrice = midPrice - book_spread / 2.0;
            double askPrice = midPrice + book_spread / 2.0;
            for (int level = 1; level <= 5; ++level) {
                bidPrice -= tick_size;
                askPrice += tick_size;
                long size = level * 10'000'000;
                os_book << "," << Price2Str(bidPrice) << "," << size << "," << Price2Str(askPrice) << "," << size;
            }
            os_book << endl;
        }

        // oscillate mid price
        if (priceIncreasing) {
            midPrice += 1.0 / 256.0;
            if (midPrice >= 101.0) {
                priceIncreasing = false;
            }
        }
        else {
            midPrice -= 1.0 / 256.0;
            if (midPrice <= 99.0) {
                priceIncreasing = true;
            }
        }

        // oscillate book_spread
        if (spreadIncreasing) {
            book_spread += 1.0 / 128.0;
            if (book_spread >= 1.0 / 32.0) {
                spreadIncreasing = false;
            }
        }
        else {
            book_spread -= 1.0 / 128.0;
            if (book_spread <= 1.0 / 128.0) {
                spreadIncreasing = true;
            }
        }

        // increase time
        t += 1000;
    }

    os_price.close();
    os_book.close();
}

// Generate trades data
void genTrades(const vector<string>& products, const string& tradeFile= "trades.txt", long long seed=12345) 
{
    vector<string> books = { "TRSY1", "TRSY2", "TRSY3" };
    vector<long> quantities = { 1000000, 2000000, 3000000, 4000000, 5000000 };
    std::ofstream os(tradeFile);
    std::mt19937 gen(seed);

    for (const auto& product : products) {
        for (int i = 0; i < 10; ++i) {
            string side = (i % 2 == 0) ? "BUY" : "SELL";
            // generate a 12 digit random trade id with number and letters
            string tradeId = GenerateRandomId(12);
            // generate random buy price 99-100 and random sell price 100-101 with given seed
            std::uniform_real_distribution<double> dist(side == "BUY" ? 99.0 : 100.0, side == "BUY" ? 100.0 : 101.0);
            double price = dist(gen);
            long quantity = quantities[i % quantities.size()];
            string book = books[i % books.size()];

            os << product << "," << tradeId << "," << Price2Str(price) << "," << book << "," << quantity << "," << side << endl;
        }
    }

    os.close();
}

// Generate inquiry data
void genInquiries(const vector<string>& products, const string& inquiryFile="inquiries.txt", long long seed=12345) 
{
    std::ofstream os(inquiryFile);
    std::mt19937 gen(seed);
    vector<long> quantities = { 1000000, 2000000, 3000000, 4000000, 5000000 };

    for (const auto& product : products) {
        for (int i = 0; i < 10; ++i) {
            string side = (i % 2 == 0) ? "BUY" : "SELL";
            // generate a 12 digit random inquiry id with number and letters
            string inquiryId = GenerateRandomId(12);
            // generate random buy price 99-100 and random sell price 100-101 with given seed
            std::uniform_real_distribution<double> dist(side == "BUY" ? 99.0 : 100.0, side == "BUY" ? 100.0 : 101.0);
            double price = dist(gen);
            long quantity = quantities[i % quantities.size()];
            string status = "RECEIVED";

            os << inquiryId << "," << product << "," << side << "," << quantity << "," << Price2Str(price) << "," << status << endl;
        }
    }
}



#endif