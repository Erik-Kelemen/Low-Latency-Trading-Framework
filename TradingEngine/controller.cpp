#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <algorithm>
#include <sqlite3.h>

#include "../Model/stock_price.h"
#include "../Profiler/performance_profiler.h"
#include "../MarketData/web_scraper.cpp"

#include "data_consumer.cpp"
#include "trading_engine.h"
#include "position_calculator.cpp"

void persistTrades(const std::vector<StockTrade>& trades);
void insertTradesToDatabase(const std::vector<StockTrade>& trades);
void calculateTradeStatistics(int& totalTrades, std::map<std::string, int>& tickerCounts);

/**
 * @class Controller
 * @brief A class that manages the trading framework.
 */
class Controller { 
private:
    TradingEngine& tradingEngine;
    double cash;
    int lookbackPeriod;
    const std::vector<std::string>& symbols;
    const std::vector<std::string>& targetDates;
    Profiler& profiler;
public:
    /**
     * @brief Constructor for the Controller class.
     * 
     * @param tradingEngine The trading engine object responsible for executing trading strategies.
     * @param cash The initial cash amount for the trading engine to trade with.
     * @param lookbackPeriod The duration, in milliseconds, for the trading engine to receive historical prices for.
     * @param symbols A reference to a constant vector of strings representing stock symbols.
     * @param targetDates A reference to a constant vector of strings representing target dates for data retrieval.
     * @param profiler The profiler object to be used for performance measurement.
     */
    Controller(TradingEngine& tradingEngine, double cash, int lookbackPeriod,
           const std::vector<std::string>& symbols, const std::vector<std::string>& targetDates,
           Profiler& profiler) 
        : tradingEngine(tradingEngine),
          cash(cash),
          lookbackPeriod(lookbackPeriod),
          symbols(symbols),
          targetDates(targetDates),
          profiler(profiler) {}
    /**
     * @brief Runs the trading framework for the specified target dates.
     *
     * This function runs the trading framework for a list of target dates. For each
     * date, it scrapes the necessary data, maintains a sliding window of historical
     * stock prices (lookbackWindow), and executes the trading strategy based on the
     * data in the window. The window size is determined by the lookback period.
     *
     * @note The function consumes new Kafka messages every 10 milliseconds and
     *       maintains a sliding window of historical data. The sliding window ensures
     *       that the data for the specified lookback period is always available for
     *       the trading strategy.
     */
    void runTradingFramework() {
        profiler.startComponent("Controller");

        double cash = this->cash;
        double currentProfitsLosses = 0.0;
        std::unordered_map<std::string, double> currentHoldings;
        for (const std::string& date : targetDates) {
            scrape(symbols, date, profiler);
            
            KafkaConsumer kafkaConsumer(profiler);
            
            std::deque<StockPrice> lookbackWindow; 
            while (true) {
                std::vector<StockPrice> newData = kafkaConsumer.consumeMessages(10);
                
                if (newData.empty())
                    break;

                lookbackWindow.erase(lookbackWindow.begin(), lookbackWindow.end() - newData.size());
                lookbackWindow.insert(lookbackWindow.end(), newData.begin(), newData.end());

                std::vector<StockTrade> trades = tradingEngine.executeTradingStrategy(lookbackWindow, cash, currentHoldings, currentProfitsLosses);
                
                persistTrades(trades);

                updateHoldingsAndCash(trades, currentHoldings, currentProfitsLosses, cash, profiler);
            }
        }
        
        int totalTrades; std::map<std::string, int> countByTicker;
        calculateTradeStatistics(totalTrades, countByTicker);
        std::cout << "Initial Cash: " << this->cash << std::endl;
        std::cout << "Final Cash: " << cash << std::endl;
        std::cout << "Change in Cash: " << cash - this->cash << std::endl;
        std::cout << "Final P&L: " << currentProfitsLosses << std::endl;
        for(auto [ticker, ct]: countByTicker)
            std::cout << ticker << ": " << ct << " trades executed" << std::endl;
        std::cout << "Total trades executed: " << totalTrades << std::endl;

        profiler.stopComponent("Controller");
    }
};
/**
 * @brief Inserts a vector of trades into the SQLite3 database stored in "trades.db". 
 *
 * @param trades The vector of StockTrade objects to insert into the database.
 */
void persistTrades(const std::vector<StockTrade>& trades) {
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
/**
 * @brief Query the database to calculate trade statistics.
 *
 * @param totalTrades Reference to store the total number of trades.
 * @param tickerCounts Reference to store the count of each ticker type.
 */
void calculateTradeStatistics(int& totalTrades, std::map<std::string, int>& tickerCounts) {
    sqlite3* db;
    int rc = sqlite3_open("trades.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    const char* selectSQL = "SELECT ticker, COUNT(*) FROM TRADES GROUP BY ticker;";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* ticker = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int count = sqlite3_column_int(stmt, 1);
        tickerCounts[std::string(ticker)] = count;
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    totalTrades = 0;
    for (const auto& entry : tickerCounts) {
        totalTrades += entry.second;
    }
}