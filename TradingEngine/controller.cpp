#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <algorithm>
#include <sqlite3.h>

#include "../model/stock_price.h"
#include "../Profiler/performance_profiler.h"
#include "data_receiver.cpp"
#include "trading_engine.cpp"
#include "position_calculator.cpp"

void insertTradesToDatabase(const std::vector<StockTrade>& trades);

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

        TradingEngine tradingEngine;
        PositionCalculator positionCalculator;

        std::vector<StockTrade> trades = tradingEngine.executeTradingStrategy(lookbackWindow, currentCash, currentHoldings, currentProfitsLosses);
        
        insertTradesToDatabase(trades);

        positionCalculator.calculatePnL(trades, currentHoldings, currentProfitsLosses, currentCash);
        this->prof->stopComponent("Controller");
        this->prof->printComponentTimes();
    }
    

}
/**
 * @brief Inserts a vector of trades into the SQLite3 database stored in "trades.db". 
 *
 * @param trades The vector of StockTrade objects to insert into the database.
 */
void insertTradesToDatabase(const std::vector<StockTrade>& trades) {
    sqlite3* db;
    int rc = sqlite3_open("trades.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS TRADES ("
                                "ticker TEXT NOT NULL,"
                                "time TEXT NOT NULL,"
                                "qty INTEGER NOT NULL);";
    rc = sqlite3_exec(db, createTableSQL, nullptr, 0, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    for (const StockTrade& trade : trades) {
        std::string insertSQL = "INSERT INTO TRADES (ticker, time, qty) VALUES ('" +
                                trade.ticker + "', '" + trade.timestamp + "', " + std::to_string(trade.qty) + ");";
        rc = sqlite3_exec(db, insertSQL.c_str(), nullptr, 0, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return;
        }
    }

    sqlite3_close(db);
}

//g++ -std=c++17 -Wall -Wextra -I/usr/local/include -o your_program your_program.cpp -lsqlite3

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