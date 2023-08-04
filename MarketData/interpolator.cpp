#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>

#include "../Profiler/performance_profiler.h"
#include "../stock_price.h"
#include "../util.h"
#include <fstream>
#include <sstream>

const std::string& exchangeFile = "exchange_prices.csv";
const std::string& interpolatedFile = "interpolated_prices.csv";

long long convertToMilliseconds(const std::string& timeString);
void interpolate(Profiler profiler);
std::vector<StockPrice> interpolateStockPrices(const std::vector<StockPrice>& historicalPrices);


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

void interpolate(Profiler profiler){
    profiler.startComponent("Interpolator");
    std::ofstream outputFile(interpolatedFile);
    
    const std::vector<StockPrice>& prices = read(exchangeFile);
    const std::vector<StockPrice>& interpolated_prices = interpolateStockPrices(prices);
    
    if (outputFile.is_open()) {
        std::streambuf* originalBuffer = std::cout.rdbuf(); 
        std::cout.rdbuf(outputFile.rdbuf()); 
        
        std::vector<StockPrice> prices;
        for(StockPrice p: interpolated_prices)
            p.print();
        std::cout.rdbuf(originalBuffer);
        
        outputFile.close();
    } else {
        std::cerr << "Error opening the file: " << interpolatedFile << std::endl;
    }
    
    profiler.stopComponent("Interpolator");
}
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

//g++ -std=c++17 -Wall -Wextra -I/home/mars/rapidjson/include -I../Profiler -I../ interpolator.cpp ../stock_price.cpp ../Profiler/performance_profiler.cpp -o interpolator -lcurl

int main() {
    Profiler prof;
    interpolate(prof);

    return 0;
}