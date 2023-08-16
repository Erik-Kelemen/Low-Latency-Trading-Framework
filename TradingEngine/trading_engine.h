#pragma once

#include <vector>
#include <string>
#include <deque>
#include <unordered_map>
#include "../Model/stock_price.h"
#include "../Model/stock_trade.h"
#include "../Profiler/performance_profiler.h"

/**
 * @class TradingEngine
 * @brief A class that implements a simple trading engine with a moving average crossover strategy.
 */
class TradingEngine {
private:
    Profiler& profiler_; ///< The profiler object for performance measurement.

public:
    /**
     * @brief Constructor to initialize the TradingEngine.
     * @param profiler The profiler object to be used for performance measurement.
     */
    TradingEngine(Profiler& profiler);

    /**
     * @brief Implements the Moving Average Crossover trading strategy.
     * @param lookbackWindow The deque of StockPrice representing the lookback window of prices.
     * @return Vector of stock tickers to buy based on the trading strategy.
     */
    std::vector<std::string> movingAverageCrossover(const std::deque<StockPrice>& lookbackWindow);

    /**
     * @brief Executes trades based on the trading strategy and available cash.
     * @param lookbackWindow The deque of StockPrice representing the lookback window of prices.
     * @param cash[in, out] The available cash for trading (updated after executing trades).
     * @return Vector of StockTrade representing the trades to make.
     */
    std::vector<StockTrade> executeTradingStrategy(const std::deque<StockPrice>& lookbackWindow, double cash,
                                                   const std::unordered_map<std::string, double>& currentHoldings,
                                                   double currentProfitsLosses);
};
