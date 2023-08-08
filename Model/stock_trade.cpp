#include "stock_trade.h"

void StockTrade::print() {
    std::cout << "Ticker: " << ticker << ", Time: " << timestamp << ", Quantity: " << qty << std::endl;
}