#include "stock_price.h"

StockPrice::StockPrice(const std::string& ticker, const std::string& time, double price)
: ticker_(ticker), time_(time), price_(price) {}

const std::string& StockPrice::getTicker() const {
    return ticker_;
}

const std::string& StockPrice::getTime() const {
    return time_;
}

double StockPrice::getPrice() const {
    return price_;
}
void StockPrice::print() const {
    std::cout << ticker_ << "," << time_ << "," << price_ << std::endl;
}
