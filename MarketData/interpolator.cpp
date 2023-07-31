#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>

struct StockPrice {
    std::string symbol;
    double price;
};

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
            // Find the historical prices surrounding the current time
            const StockPrice& prevPrice = historicalPrices[currentIndex];
            const StockPrice& nextPrice = historicalPrices[currentIndex + 1];

            if (currentTime >= std::stoll(prevPrice.symbol)) {
                double timeFraction = static_cast<double>(currentTime - std::stoll(prevPrice.symbol)) /
                                      (std::stoll(nextPrice.symbol) - std::stoll(prevPrice.symbol));
                double interpolatedPrice = prevPrice.price +
                                           timeFraction * (nextPrice.price - prevPrice.price);

                // Add some random variation to the interpolated price
                double priceDelta = priceDeltaDistribution(rng);
                interpolatedPrice += priceDelta;

                interpolatedPrices.push_back({std::to_string(currentTime), interpolatedPrice});
            } else {
                // If currentTime is before the first historical price, use the first historical price
                interpolatedPrices.push_back({std::to_string(currentTime), prevPrice.price});
            }

            // Move to the next time step
            currentTime += millisecondsInterval;
        } else {
            // If there are no more historical prices, stop interpolating
            break;
        }

        // Move to the next historical price if currentTime exceeds the next price's time
        if (currentTime > std::stoll(nextPrice.symbol)) {
            currentIndex++;
        }
    }

    return interpolatedPrices;
}

int main() {
    // Assume historicalPrices contains the output of the web scraper (StockPrice objects)
    std::vector<StockPrice> historicalPrices = {
        {"34200000", 2800.0},  // 9:30 AM
        {"36000000", 2810.0},  // 9:40 AM
        // ... Add more historical prices here ...
        {"57600000", 2805.0}   // 4:00 PM
    };

    std::vector<StockPrice> interpolatedPrices = interpolateStockPrices(historicalPrices);

    // Print the interpolated prices for the single day
    for (const auto& price : interpolatedPrices) {
        std::cout << "Time: " << price.symbol << ", Price: " << price.price << std::endl;
    }

    return 0;
}