#include "stock_price.h"

StockPrice::StockPrice(const std::string& ticker_, const std::string& time_, double price_)
: ticker(ticker_), time(time_), price(price_) {}

void StockPrice::print() const {
    std::cout << ticker << "," << time << "," << price << std::endl;
}
