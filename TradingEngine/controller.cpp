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
#include "trading_engine.cpp"
#include "position_calculator.cpp"
namespace plt = matplotlibcpp;

void insertTradesToDatabase(const std::vector<StockTrade>& trades);
void createAndSaveGraph(const std::vector<double>& timePoints, const std::vector<double>& values, const std::string& title, const std::string& yLabel, const std::string& filename);


/**
 * @class Controller
 * @brief A class that manages the trading framework.
 */
class Controller { 
private:
    Profiler profiler;
    std::vector<std::string>& symbols;
    std::vector<std::string>& targetDates;
    bool graph;
    double cash;
    const std::string brokerAddr = "localhost:9092";
    const std::string topicName = "PRICES";
public:
    /**
     * @brief Constructor for the Controller class.
     * 
     * @param cash The initial cash amount for the trading engine to trade with.
     * @param lookbackPeriod The duration, in milliseconds, for the trading engine to receive historical prices for.
     * @param symbols A reference to a constant vector of strings representing stock symbols.
     * @param targetDates A reference to a constant vector of strings representing target dates for data retrieval.
     * @param profiler The profiler object to be used for performance measurement.
     */
    Controller(const double cash, const int lookbackPeriod, const std::vector<std::string>& symbols,
                const std::vector<std::string>& targetDates, Profiler profiler, bool graph=true) {
        this->cash = cash;
        this->lookbackPeriod = lookbackPeriod;
        this->symbols = symbols;
        this->targetDates = targetDates;
        this->profiler = profiler;
        this->graph = graph;
    }
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
        this->profiler->startComponent("Controller");

        std::vector<int> timePoints;
        std::vector<double> cashValues;
        std::vector<double> pnlValues;
        std::vector<double> holdingsValues;

        double cash = this->cash;
        double currentProfitsLosses = 0.0;
        std::unordered_map<std::string, double> currentHoldings;
        for (const std::string& date : this->targetDates) {
            scrape(this->symbols, date, this->profiler);
            
            KafkaConsumer kafkaConsumer(this->brokerAddr, this->topicName);
            
            std::deque<StockPrice> lookbackWindow; 
            while (true) {
                std::vector<StockPrice> newData = kafkaConsumer.consumeMessages(10);
                
                if (newData.empty())
                    break;

                lookbackWindow.erase(lookbackWindow.begin(), lookbackWindow.end() - newData.size());
                lookbackWindow.insert(lookbackWindow.end(), newData.begin(), newData.end());

                std::vector<StockTrade> trades = tradingEngine.executeTradingStrategy(lookbackWindow, cash, currentHoldings, currentProfitsLosses);

                persistTrades(trades);

                updateHoldingsAndCash(trades, currentHoldings, currentProfitsLosses, cash);

                if(this->graph){
                    timePoints.push_back(newData[0].time);
                    cashValues.push_back(cash);
                    pnlValues.push_back(currentProfitsLosses);
                    holdingsValues.push_back(currentHoldings);
                }
                
            }
        }
        if(this->graph){
            createAndSaveGraph(timePoints, cashValues, "Cash Over Time", "Cash", "cash.png");
            createAndSaveGraph(timePoints, pnlValues, "P&L Over Time", "P&L", "pnl.png");
            createAndSaveGraph(timePoints, holdingsValues, "Stock Holdings Over Time", "Holdings", "holdings.png");
        }
        this->profiler->stopComponent("Controller");
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
void createAndSaveGraph(const std::vector<double>& timePoints, const std::vector<double>& values, 
                        const std::string& title, const std::string& yLabel, const std::string& filename) {
    plt::plot(timePoints, values);
    plt::title(title);
    plt::xlabel("Time");
    plt::ylabel(yLabel);
    plt::save("imgs/" + filename);
    plt::clf();
}