#ifndef STOCK_PRICE_H
#define STOCK_PRICE_H

#include <string>
#include <chrono>
#include <iostream>

class StockPrice {
public:
    StockPrice(const std::string& ticker, const std::string& time, const double price);

    // Getters for ticker, time, and price
    const std::string& getTicker() const;
    const std::string& getTime() const;
    double getPrice() const;
    void print() const;
private:
    std::string ticker_;
    std::string time_;
    double price_;
};

#endif // STOCK_PRICE_H
