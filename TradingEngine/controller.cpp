#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <algorithm>
#include <sqlite3.h>

#include "../Model/stock_price.h"
#include "../Profiler/performance_profiler.h"
#include "../MarketData/web_scraper.cpp"
#include "../MarketData/interpolator.cpp"
#include "../MarketData/data_publisher.cpp"

#include "data_receiver.cpp"
#include "trading_engine.cpp"
#include "position_calculator.cpp"
#include <cpp_redis/cpp_redis>

void insertTradesToDatabase(const std::vector<StockTrade>& trades);

/**
 * @class Controller
 * @brief A class that manages the trading framework.
 */
class Controller { 
private:
    Profiler profiler;
    std::vector<std::string>& symbols;
    std::vector<std::string>& targetDates;
public:
    Controller(std::vector<std::string>& symbols, std::vector<std::string>& targetDates, Profiler profiler){
        this->symbols = symbols;
        this->targetDates = targetDates;
        this->profiler = profiler;
    }
    /**
     * @brief Function to run the trading framework.
     */
    void runTradingFramework(){
        this->profiler->startComponent("Controller");
        
        for(std::string date: this->targetDates){
            double currentCash = 1000000.0;
            TradingEngine tradingEngine(currentCash);

            scrape(this->symbols, date, this->profiler);
            interpolate(this->profiler);
            publish(this->profiler);

            std::string brokerAddr = "localhost:9092";
            std::string topicName = "PRICES";

            KafkaConsumer kafkaConsumer(brokerAddr, topicName);

            int lookbackPeriod = 30000; // 30 seconds in milliseconds
            std::vector<StockPrice> lookbackWindow = kafkaConsumer.consumeMessages(lookbackPeriod);
            
            
            std::vector<StockPrice> currentHoldings;
            double currentProfitsLosses = 0.0;

            std::vector<StockTrade> trades = tradingEngine.executeTradingStrategy(lookbackWindow, currentCash, currentHoldings, currentProfitsLosses);
            
            insertTradesToDatabase(trades);

            calculatePnL(trades, currentHoldings, currentProfitsLosses, currentCash);
        }
        this->profiler->stopComponent("Controller");
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