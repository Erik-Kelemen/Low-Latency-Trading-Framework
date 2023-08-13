#include <vector>
#include <string>
#include <chrono>
#include "../Model/stock_price.h"
#include "../Profiler/performance_profiler.h"

/**
 * @class TradingEngine
 * @brief A class that implements a simple trading engine with a moving average crossover strategy.
 */
class TradingEngine {
private:
    double cash_;
    std::vector<StockPrice> holdings_; ///< Vector to store the stocks and their quantities held.
    double profitsLosses_; ///< Total profits and losses from executed trades.

public:
    /**
     * @brief Constructor to initialize the TradingEngine.
     * @param initialCash The initial amount of cash available for trading.
     */
    TradingEngine(double initialCash)
        : cash_(initialCash), profitsLosses_(0.0) {}

    /**
     * @brief Function to implement the Moving Average Crossover trading strategy.
     * @param lookbackWindow The vector of StockPrice representing the lookback window of prices.
     * @return A vector of stock tickers to buy based on the trading strategy.
     */
    std::vector<std::string> movingAverageCrossover(const std::vector<StockPrice>& lookbackWindow) {
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

    /**
     * @brief Function to execute trades based on the trading strategy and available cash.
     * @param lookbackWindow The vector of StockPrice representing the lookback window of prices.
     */
    void executeTrades(const std::vector<StockPrice>& lookbackWindow) {
        std::vector<std::string> stocksToBuy = movingAverageCrossover(lookbackWindow);

        for (const auto& stock : stocksToBuy) {
            double maxQuantity = cash_ / lookbackWindow.back().price;
            double quantityToBuy = std::min(maxQuantity, 1000.0);

            double cost = quantityToBuy * lookbackWindow.back().price;
            cash_ -= cost;

            holdings_.push_back({stock, quantityToBuy});
            profitsLosses_ -= cost;
        }
    }

    /**
     * @brief Function to get the current cash.
     * @return The current available cash for trading.
     */
    double getCurrentCash() const {
        return cash_;
    }

    /**
     * @brief Function to get the current holdings.
     * @return A constant reference to the vector of StockPrice representing the current holdings.
     */
    const std::vector<StockPrice>& getCurrentHoldings() const {
        return holdings_;
    }

    /**
     * @brief Function to get the current profits and losses.
     * @return The total profits and losses from executed trades.
     */
    double getCurrentProfitsLosses() const {
        return profitsLosses_;
    }
};
