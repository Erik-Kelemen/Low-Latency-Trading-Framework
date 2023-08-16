#include "trading_engine.h"

TradingEngine::TradingEngine(Profiler& profiler)
    : profiler_(profiler) {}

std::vector<std::string> TradingEngine::movingAverageCrossover(const std::deque<StockPrice>& lookbackWindow) {
    std::vector<std::string> stocksToBuy;

    double ma30 = 0.0;
    for (const auto& price : lookbackWindow) {
        ma30 += price.price;
    }
    ma30 /= lookbackWindow.size();

    for (const auto& price : lookbackWindow) {
        if (price.price < ma30) {
            stocksToBuy.push_back(price.ticker);
        }
    }

    return stocksToBuy;
}

std::vector<StockTrade> TradingEngine::executeTradingStrategy(const std::deque<StockPrice>& lookbackWindow, double cash,
                                                              const std::unordered_map<std::string, double>& currentHoldings,
                                                              double currentProfitsLosses) {
    profiler_.startComponent("TradingEngine");

    std::vector<StockTrade> trades;

    std::vector<std::string> stocksToBuy = movingAverageCrossover(lookbackWindow);

    for (const auto& stock : stocksToBuy) {
        double maxQuantity = cash / lookbackWindow.back().price;
        double quantityToBuy = std::min(maxQuantity, 1000.0);

        double cost = quantityToBuy * lookbackWindow.back().price;
        cash -= cost;

        trades.push_back({stock, lookbackWindow.back().time, static_cast<size_t>(quantityToBuy)});
    }

    profiler_.stopComponent("TradingEngine");

    return trades;
}
