/**
* utility.hpp
* Contain some utility functions:
*   Str2Price: convert string to price value
*   Price2Str: convert price value to string
*   IdGenerator: generate id
*   GetPV01Value: get pv01 value by cusip
* @Yunze Sun
*/
#pragma once
#ifndef utility_h
#define utility_h

#include <iostream>
#include <string>
#include <iomanip>
#include "products.hpp"

using namespace std;

// Str2Price: convert string to price value
double Str2Price(string s) {
    int len = s.size();
    s[len - 1] = (s[len - 1] == '+') ? '4' : s[len - 1];
    // fractional notation transformation
    int a = stoi(s.substr(0, len - 4));
    double b = stoi(s.substr(len - 3, 2)) / 32.;
    double c = stoi(s.substr(len - 1, 1)) / 256.;
    return a + b + c;
}

// Price2Str: convert price value to string
string Price2Str(double price) 
{
    int a = floor(price);
    double b = price - a;
    int xy = floor(b * 32);
    int z = static_cast<int>((b * 256)) % 8;
    // be careful about corner cases
    return to_string(a) + "-" + (xy < 10 ? "0" : "") + to_string(xy) + (z == 4 ? "+" : to_string(z));
}

// IdGenerator: generate id
string IdGenerator(long index, int length)
{
    std::ostringstream ss;
    ss << std::setw(length) << std::setfill('0') << index;
    return ss.str();
}

double GetPV01Value(string _cusip)
{
    double _pv01 = 0;
    if (_cusip == "9128283H1") _pv01 = 0.01948992;
    if (_cusip == "9128283L2") _pv01 = 0.02865304;
    if (_cusip == "912828M80") _pv01 = 0.04581119;
    if (_cusip == "9128283J7") _pv01 = 0.06127718;
    if (_cusip == "9128283F5") _pv01 = 0.08161449;
    if (_cusip == "912810TM0") _pv01 = 0.11707914;
    if (_cusip == "912810RZ3") _pv01 = 0.15013155;
    return _pv01;
}

// get bond by its cusip
Bond GetBond(string _cusip)
{
    if (_cusip == "9128283H1") return Bond("9128283H1", CUSIP, "US2Y", 0.01750, from_string("2019/11/30"));
    if (_cusip == "9128283L2") return Bond("9128283L2", CUSIP, "US3Y", 0.01875, from_string("2020/12/15"));
    if (_cusip == "912828M80") return Bond("912828M80", CUSIP, "US5Y", 0.02000, from_string("2022/11/30"));
    if (_cusip == "9128283J7") return Bond("9128283J7", CUSIP, "US7Y", 0.02125, from_string("2024/11/30"));
    if (_cusip == "9128283F5") return Bond("9128283F5", CUSIP, "US10Y", 0.02250, from_string("2027/12/15"));
    if (_cusip == "912810TM0") return Bond("912810TM0", CUSIP, "US20Y", 0.02400, from_string("2037/12/15"));
    if (_cusip == "912810RZ3") return Bond("912810RZ3", CUSIP, "US30Y", 0.02750, from_string("2047/12/15"));
}
#endif