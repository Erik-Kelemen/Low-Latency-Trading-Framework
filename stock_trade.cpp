#include "stock_trade.h"

void StockTrade::print() const {
    std::cout << "Ticker: " << ticker << ", Time: " << timestamp << ", Quantity: " << qty << std::endl;
}