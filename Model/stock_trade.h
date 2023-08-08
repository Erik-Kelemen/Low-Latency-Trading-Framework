#pragma once

#ifndef STOCKTRADE_H
#define STOCKTRADE_H

#include <iostream>
#include <string>

struct StockTrade {
    std::string ticker;   // Stock ticker symbol
    std::string timestamp; // Timestamp of the trade
    size_t qty;          // Quantity of stocks traded

    StockTrade(const std::string& ticker, const std::string& timestamp, size_t qty)
        : ticker(ticker), timestamp(timestamp), qty(qty) {}

    // Print method to display the trade details
    void print();
};

#endif // STOCKTRADE_H
