#pragma once

#ifndef STOCK_PRICE_H
#define STOCK_PRICE_H

#include <string>
#include <chrono>
#include <iostream>

struct StockPrice {
    StockPrice(const std::string& ticker_, const std::string& time_, const double price_);

    std::string ticker;
    std::string time;
    double price;
    void print() const;
};

#endif 
