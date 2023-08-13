#include <vector>
#include "../Model/stock_price.h"
#include "../Model/stock_trade.h"
/**
 * @brief Calculate the profits and losses and update the holdings after executing trades.
 * 
 * This class is responsible for calculating the profits and losses and updating the current holdings
 * after executing trades based on the trading strategy.
 * 
 * @param trades The vector of StockTrade objects representing the trades executed based on the trading strategy.
 * @param holdings The vector of StockPrice objects representing the current holdings of stocks.
 * @param profitsLosses The reference to a double representing the current profits and losses.
 * @param cash The reference to a double representing the current available cash.
 */
void updateHoldingsAndCash(const std::vector<StockTrade>& trades, std::unordered_map<std::string, double>& holdings, double& profitsLosses, double& cash) {
    for (const StockTrade& trade : trades) {
        double currentQuantity = 0.0;
        auto it = holdings.find(trade.ticker);
        if (it != holdings.end()) {
            currentQuantity = it->second;
        }

        double newQuantity = currentQuantity + trade.qty;
        if (newQuantity <= 0) {
            holdings.erase(trade.ticker);
        } else {
            holdings[trade.ticker] = newQuantity;
        }

        cash -= trade.qty * trade.price;
        profitsLosses += trade.qty * (trade.price - trade.averagePrice);
    }
}