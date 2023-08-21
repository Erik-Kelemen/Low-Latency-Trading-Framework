#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>
#include <future>
#include <algorithm>

#include "../Profiler/performance_profiler.h"
#include "../Model/stock_price.h"
#include "../Model/util.h"
#include "data_publisher.cpp"


long long convertToMilliseconds(const std::string& timeString);
void interpolate(Profiler profiler);
std::vector<StockPrice> interpolateStockPrices(const std::vector<StockPrice>& historicalPrices);
std::vector<StockPrice> interpolateStockPricesMultiThread(const std::vector<StockPrice>& historicalPrices, std::vector<StockPrice>& interpolatedPrices)
void interpolateSegment(const std::vector<StockPrice>& segment, std::vector<StockPrice>& resultSegment);
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
void interpolate(const std::vector<StockPrice>& prices, Profiler& profiler, bool read_from_file = false, bool persist = false){
    profiler.startComponent("Interpolator");
    
    if (read_from_file) 
        read(exchangeFile);
    std::vector<StockPrice> interpolatedPrices;
    interpolateStockPricesMultiThread(prices, interpolatedPrices);
    
    if (persist)
        write("ticker,jsonData,targetDate", interpolatedFile, interpolatedPrices);
    
    profiler.stopComponent("Interpolator");

    KafkaPublisher kafkaPublisher(profiler);
    kafkaPublisher.publish(interpolatedPrices);
}

/**
 * Interpolates the stock prices between historical data points using multithreading.
 * This function divides the historical prices into segments and performs interpolation on each segment
 * concurrently using std::async. The interpolated segments are then merged and sorted based on time.
 *
 * @param historicalPrices A vector of StockPrice objects containing historical data points.
 * @return A vector of StockPrice objects representing the interpolated stock prices.
 */
std::vector<StockPrice> interpolateStockPricesMultiThread(const std::vector<StockPrice>& historicalPrices,
                                                        std::vector<StockPrice>& interpolatedPrices) {
    const size_t numSegments = 14;
    const size_t segmentSize = historicalPrices.size() / numSegments;

    std::vector<std::thread> threads;

    for (size_t i = 0; i < numSegments; i++) {
        size_t startIdx = i * segmentSize;
        size_t endIdx = (i == numSegments - 1) ? historicalPrices.size() : startIdx + segmentSize;
        std::vector<StockPrice> segment(historicalPrices.begin() + startIdx,
                                        historicalPrices.begin() + endIdx);

        threads.emplace_back([&segment, &interpolatedPrices]() {
            std::vector<StockPrice> resultSegment;
            interpolateSegment(segment, resultSegment);

            std::lock_guard<std::mutex> lock(interpolatedPricesMutex);
            interpolatedPrices.insert(interpolatedPrices.end(), resultSegment.begin(), resultSegment.end());
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::sort(interpolatedPrices.begin(), interpolatedPrices.end(),
              [](const StockPrice& a, const StockPrice& b) {
                  return convertToMilliseconds(a.time) < convertToMilliseconds(b.time);
              });
}


/**
 * Interpolates the stock prices between historical data points.
 * @param historicalPrices A vector of StockPrice objects containing historical data points.
 * @return A vector of StockPrice objects representing the interpolated stock prices.
 */
void interpolateSegment(const std::vector<StockPrice>& segment,
                        std::vector<StockPrice>& resultSegment) {
    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> priceDeltaDistribution(-0.0005, 0.0005);

    const int millisecondsInterval = 10;

    for (size_t currentIndex = 0; currentIndex < segment.size() - 1; currentIndex++) {
        const StockPrice& prevPrice = segment[currentIndex];
        const StockPrice& nextPrice = segment[currentIndex + 1];

        int startTime = convertToMilliseconds(prevPrice.time);
        int endTime = convertToMilliseconds(nextPrice.time);

        for (int currentTime = startTime; currentTime < endTime; currentTime += millisecondsInterval) {
            double timeFraction = static_cast<double>(currentTime - startTime) /
                                  (endTime - startTime);
            double interpolatedPrice = prevPrice.price +
                                       timeFraction * (nextPrice.price - prevPrice.price);

            double priceDelta = priceDeltaDistribution(rng);
            interpolatedPrice += priceDelta;

            resultSegment.push_back(StockPrice(nextPrice.ticker, std::to_string(currentTime), interpolatedPrice));
        }
    }
}