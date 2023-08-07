#include <iostream>
#include <string>
#include <vector>
#include <librdkafka/rdkafkacpp.h>
#include <thread>
#include <algorithm>

#include "../model/stock_price.h"
#include "../Profiler/performance_profiler.h"
#include "data_consumer.cpp"
#include "trading_engine.cpp"
#include "position_calculator.cpp"

/**
 * @class Controller
 * @brief A class that manages the trading framework.
 */
class Controller { 
    Profiler* prof;
public:
    /**
     * @brief Function to run the trading framework.
     * This function consumes stock price data, executes the trading strategy,
     * and calculates profits & losses.
     */
    void runTradingFramework(){
        this->prof = new Profiler();
        this->prof->startComponent("Controller");
        std::string brokerAddr = "localhost:9092";
        std::string topicName = "PRICES";
        KafkaConsumer kafkaConsumer(brokerAddr, topicName);

        int lookbackPeriod = 30000; // 30 seconds in milliseconds
        std::vector<StockPrice> lookbackWindow = kafkaConsumer.consumeMessages(lookbackPeriod);

        double currentCash = 1000000.0;
        std::vector<StockPrice> currentHoldings;
        double currentProfitsLosses = 0.0;

        // Initialize the TradingEngine and PositionCalculator objects
        TradingEngine tradingEngine;
        PositionCalculator positionCalculator;

        // Execute the trading strategy and get the trades to make
        std::vector<StockTrade> trades = tradingEngine.executeTradingStrategy(lookbackWindow, currentCash, currentHoldings, currentProfitsLosses);

        // Calculate the new P&L and remaining cash after executing the trades
        positionCalculator.calculatePnL(trades, currentHoldings, currentProfitsLosses, currentCash);
        this->prof->stopComponent("Controller");
        this->prof->printComponentTimes();
    }
}

int main() {
    std::string brokerAddr = "localhost:9092"; 
    std::string topicName = "stock_prices";
    KafkaConsumer kafkaConsumer(brokerAddr, topicName);

    int lookbackPeriod = 30000; // 30 seconds in milliseconds
    std::vector<StockPrice> lookbackWindow = kafkaConsumer.consumeMessages(lookbackPeriod);

    double currentCash = 1000000.0; 
    std::vector<StockPrice> currentHoldings; 
    double currentProfitsLosses = 0.0;


    return 0;
}