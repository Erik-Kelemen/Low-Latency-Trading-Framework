#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>

#include "../Profiler/performance_profiler.h"
#include "../model/stock_price.h"
#include "../model/util.h"

const std::string& exchangeFile = "exchange_prices.csv";
const std::string& interpolatedFile = "interpolated_prices.csv";


long long convertToMilliseconds(const std::string& timeString);
void interpolate(Profiler profiler);
std::vector<StockPrice> interpolateStockPrices(const std::vector<StockPrice>& historicalPrices);

/**
 * Converts a date-time string to milliseconds since midnight.
 * @param dateTimeString The input date-time string in the format "yyyy-MM-dd HH:mm:ss".
 * @return The number of milliseconds since midnight represented by the input date-time string.
 */
long long convertToMilliseconds(const std::string& dateTimeString) {
    const std::string dateTime = dateTimeString.substr(10);
    std::istringstream iss(dateTime);
    int hours, minutes, seconds;
    char delimiter;
    if (iss >> hours >> delimiter >> minutes >> delimiter >> seconds) {
        long long totalMilliseconds = (hours * 3600 + minutes * 60 + seconds) * 1000;
        return totalMilliseconds;
    } else {
        std::cerr << "Invalid time format: " << dateTime << std::endl;
        return -1;
    }
}

/**
 * Interpolates the stock prices between historical data points.
 * @param profiler The Profiler object to measure performance.
 */
void interpolate(Profiler profiler){
    profiler.startComponent("Interpolator");
    
    const std::vector<StockPrice>& prices = read(exchangeFile);
    const std::vector<StockPrice>& interpolatedPrices = interpolateStockPrices(prices);
    
    write("ticker,jsonData,targetDate", interpolatedFile, interpolatedPrices);
    
    profiler.stopComponent("Interpolator");
}

/**
 * Interpolates the stock prices between historical data points.
 * @param historicalPrices A vector of StockPrice objects containing historical data points.
 * @return A vector of StockPrice objects representing the interpolated stock prices.
 */
std::vector<StockPrice> interpolateStockPrices(const std::vector<StockPrice>& historicalPrices) {
    std::vector<StockPrice> interpolatedPrices;
    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> priceDeltaDistribution(-0.0005, 0.0005);

    const int millisecondsInterval = 10;

    long long startTime = 34200000;  // 9:30 AM in milliseconds (Market opening time)
    long long endTime = 57600000;    // 4:00 PM in milliseconds (Market closing time)

    for(size_t currentIndex = 0; currentIndex < historicalPrices.size() - 1; currentIndex++){

        const StockPrice& prevPrice = historicalPrices[currentIndex];
        const StockPrice& nextPrice = historicalPrices[currentIndex + 1];

        long long startTime = convertToMilliseconds(prevPrice.time);
        long long endTime = convertToMilliseconds(nextPrice.time);
        
        std::cout << prevPrice.time << ' ' << nextPrice.time << std::endl;

        for(long long currentTime = startTime; currentTime < endTime; currentTime += millisecondsInterval){
            double timeFraction = static_cast<double>(currentTime - startTime) /
                                (endTime - startTime);
            double interpolatedPrice = prevPrice.price +
                                    timeFraction * (nextPrice.price - prevPrice.price);

            double priceDelta = priceDeltaDistribution(rng);
            interpolatedPrice += priceDelta;

            interpolatedPrices.push_back(StockPrice(nextPrice.ticker, std::to_string(currentTime), interpolatedPrice));
            std::cout << currentTime << std::endl;
        }
    }
    return interpolatedPrices;
}

//g++ -std=c++17 -Wall -Wextra -I./home/mars/Low-Latency-Trading-Framework interpolator.cpp ../stock_price.cpp ../util.cpp ../Profiler/performance_profiler.cpp -o interpolator

int main() {
    Profiler prof;
    interpolate(prof);

    return 0;
}