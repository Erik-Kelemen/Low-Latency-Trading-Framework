#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>

#include "../Profiler/performance_profiler.h"
#include "../stock_price.h"

#include <fstream>
#include <sstream>

std::vector<std::string> splitStringByComma(const std::string& input) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string item;

    while (std::getline(ss, item, ',')) {
        result.push_back(item);
    }

    return result;
}

class Interpolator{
private:
    const std::string& cmp_name;
    Profiler profiler;
public:
    
    Interpolator(Profiler& profiler){
        this->profiler = profiler;
    }

    void interpolate(){
        this->profiler.startComponent("Interpolator");
        std::ofstream outputFile("interpolated_prices.csv");
        
        const std::vector<StockPrice> prices = this->read();
        const std::vector<StockPrice>& interpolated_prices = this->interpolateStockPrices(prices);
        
        if (outputFile.is_open()) {
            std::streambuf* originalBuffer = std::cout.rdbuf(); 
            std::cout.rdbuf(outputFile.rdbuf()); 
            
            std::vector<StockPrice> prices;
            for(StockPrice p: interpolated_prices)
                p.print();
            std::cout.rdbuf(originalBuffer);
            
            outputFile.close();
            
        } else {
            std::cerr << "Error opening the file!" << std::endl;
        }
        
        profiler.stopComponent("Interpolator");
    }
    const std::vector<StockPrice> read(){
        const std::string filename = "exchange_prices.csv"; //read output of the web_scraper
        std::vector<StockPrice> rows;
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return rows;
        }

        while (std::getline(file, line)) {
            std::vector<std::string> line = splitStringByComma(line);
            std::string ticker, time = line[0], line[1];
            double price = line[2];
            rows.push_back(StockPrice(ticker, time, price));
        }
        file.close();
        return rows;
    }
    std::vector<StockPrice> interpolateStockPrices(const std::vector<StockPrice>& historicalPrices) {
        
        std::vector<StockPrice> interpolatedPrices;
        std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<double> priceDeltaDistribution(-0.5, 0.5);

        // Interpolate prices every 10 milliseconds
        const int millisecondsInterval = 10;

        // Start and end time for the single day
        int startTime = 34200000;  // 9:30 AM in milliseconds (Market opening time)
        int endTime = 57600000;    // 4:00 PM in milliseconds (Market closing time)

        int currentTime = startTime;
        size_t currentIndex = 0;

        while (currentTime <= endTime) {
            if (currentIndex < historicalPrices.size() - 1) {
                const StockPrice& prevPrice = historicalPrices[currentIndex];
                const StockPrice& nextPrice = historicalPrices[currentIndex + 1];

                if (currentTime >= std::stoll(prevPrice.symbol)) {
                    double timeFraction = static_cast<double>(currentTime - std::stoll(prevPrice.symbol)) /
                                        (std::stoll(nextPrice.symbol) - std::stoll(prevPrice.symbol));
                    double interpolatedPrice = prevPrice.price +
                                            timeFraction * (nextPrice.price - prevPrice.price);

                    double priceDelta = priceDeltaDistribution(rng);
                    interpolatedPrice += priceDelta;

                    interpolatedPrices.push_back(StockPrice(ticker, std::to_string(currentTime), interpolatedPrice));
                } else {
                    interpolatedPrices.push_back(StockPrice{ticker, std::to_string(currentTime), prevPrice.price});
                }

                currentTime += millisecondsInterval;
            } else {
                break;
            }

            if (currentTime > std::stoll(nextPrice.symbol)) {
                currentIndex++;
            }
        }
        
        return interpolatedPrices;
    }
};

//g++ -std=c++17 -Wall -Wextra -I/home/mars/rapidjson/include -I../Profiler -I../ interpolator.cpp ../stock_price.cpp ../Profiler/performance_profiler.cpp -o interpolator -lcurl

int main() {
    Profiler prof;
    Interpolator interpolator(prof);
    interpolator.interpolate();

    return 0;
}