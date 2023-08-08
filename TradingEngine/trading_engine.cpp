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
    double cash_; ///< Current available cash for trading.
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

        // Calculate the 30-second moving average
        double ma30 = 0.0;
        for (const auto& price : lookbackWindow) {
            ma30 += price.price;
        }
        ma30 /= lookbackWindow.size();

        // Decide which stocks to buy based on the crossover strategy
        for (const auto& price : lookbackWindow) {
            // You can implement your own criteria for stock selection here.
            // For this example, let's assume we buy stocks whose current price is below the 30-second moving average.
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
            // Calculate the maximum quantity of stocks that can be bought with the available cash
            double maxQuantity = cash_ / lookbackWindow.back().price;
            // Buy only a portion of the stocks if the available cash is not sufficient for all stocks
            double quantityToBuy = std::min(maxQuantity, 1000.0); // Buying a maximum of 1000 stocks

            double cost = quantityToBuy * lookbackWindow.back().price;
            cash_ -= cost;

            // Update the holdings and profits & losses
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


//g++ -std=c++17 -Wall -Wextra -I/home/mars/rapidjson/include -I../Profiler -I../ interpolator.cpp ../stock_price.cpp ../Profiler/performance_profiler.cpp -o interpolator -lcurl

int main() {
    double initialCash = 1000000.0;
    TradingEngine tradingEngine(initialCash);

    // Execute trades based on the trading strategy and available cash
    tradingEngine.executeTrades(lookbackWindow);

    // Get the current cash, holdings, and profits & losses
    double currentCash = tradingEngine.getCurrentCash();
    const std::vector<StockPrice>& currentHoldings = tradingEngine.getCurrentHoldings();
    double currentProfitsLosses = tradingEngine.getCurrentProfitsLosses();

    // Print the results (you can customize this based on your needs)
    std::cout << "Current Cash: " << currentCash << std::endl;
    std::cout << "Current Holdings: " << std::endl;
    for (const auto& holding : currentHoldings) {
        std::cout << "Symbol: " << holding.symbol << ", Quantity: " << holding.price << std::endl;
    }
    std::cout << "Current Profits & Losses: " << currentProfitsLosses << std::endl;

    return 0;
}
