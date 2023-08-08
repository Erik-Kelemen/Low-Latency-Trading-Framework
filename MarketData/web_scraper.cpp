#include <string>
#include <curl/curl.h>
#include <rapidjson/document.h>

#include "../Profiler/performance_profiler.h"
#include "../Model/stock_price.h" // Include the StockPrice header
#include "../Model/util.h"
#include <vector>

#include <fstream>
#include <cpp_redis/cpp_redis>
// /usr/local/include/cpp_redis/includes/cpp_redis/cpp_redis

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);
void scrape(std::vector<std::string> symbols, std::string targetDate);
std::string fetchStockData(const std::string& symbol);
std::vector<StockPrice> parseStockData(const std::string ticker, const std::string& jsonData, const std::string& targetDate);

const std::string BASE_URL = "https://www.alphavantage.co/query?";
const std::string API_KEY = "ALQU3SWWYFF7QHXA"; //<--- replace with your own Alpha Vantage API Key
const std::string interval = "1min";

/**
 * @brief Callback function used by libcurl to write fetched data into a std::string.
 *
 * @param contents Pointer to the data received from the HTTP request.
 * @param size Size of each data element.
 * @param nmemb Number of data elements.
 * @param response Pointer to the std::string to store the fetched data.
 * @return Total size of the data received.
 */
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

/**
 * @brief Scrapes stock data for a given list of symbols and target date.
 *
 * @param symbols Vector of stock symbols to scrape data for.
 * @param targetDate The target date for which data is to be scraped.
 * @param profiler Profiler to measure performance.
 */
void scrape(std::vector<std::string> symbols, std::string targetDate, Profiler profiler){
    profiler.startComponent("Web Scraper");
    
    std::vector<StockPrice> prices;

    for (const std::string& symbol : symbols) {
        std::string jsonData = fetchStockData(symbol);
        std::vector<StockPrice> cur_prices = parseStockData(symbol, jsonData, targetDate);
        prices.insert(prices.end(), cur_prices.begin(), cur_prices.end());
    }
    
    write("ticker,date,price", "exchange_prices.csv", prices);
    
    profiler.stopComponent("Web Scraper");
}

/**
 * @brief Fetches stock data for a given symbol from the Alpha Vantage API.
 *
 * @param symbol The stock symbol to fetch data for.
 * @return The fetched stock data as a JSON string.
 */
std::string fetchStockData(const std::string& symbol) {
    std::string url = BASE_URL + "function=TIME_SERIES_INTRADAY&symbol=" + symbol + "&apikey=" + API_KEY + "&interval=" + interval + "&extended_hours=false&outputsize=full";
    
    std::string response;
    cpp_redis::client client;
    client.connect("localhost", 6379);
    if (client.exists(symbol).get()) {
        client.get(symbol, [&response](cpp_redis::reply& reply) {
            if (reply.is_string()) {
                response = reply.as_string();
            }
        });
    } else {    
        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            CURLcode res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                std::cerr << "Error: " << curl_easy_strerror(res) << std::endl;
            }

            curl_easy_cleanup(curl);
        }
    }
    client.disconnect
    return response;
}

/**
 * @brief Parses the fetched stock data and extracts relevant information.
 *
 * @param ticker The stock symbol.
 * @param jsonData The JSON data received from the API.
 * @param targetDate The target date to filter data for.
 * @return A vector of StockPrice objects containing the parsed data.
 */
std::vector<StockPrice> parseStockData(const std::string ticker, const std::string& jsonData, const std::string& targetDate) {
    std::vector<StockPrice> stockData;

    rapidjson::Document document;
    document.Parse(jsonData.c_str());

    if (document.HasParseError()) {
        std::cerr << "JSON parsing error!" << std::endl;
        return stockData;
    }

    const rapidjson::Value& timeSeries = document["Time Series (1min)"];

    for (auto it = timeSeries.MemberBegin(); it != timeSeries.MemberEnd(); ++it) {
        const std::string& date = it->name.GetString();
        if(date.substr(0, 10) == targetDate){
            const double openPrice = std::stod(it->value["1. open"].GetString());
            StockPrice stockPrice(ticker, date, openPrice);
            stockData.push_back(stockPrice);
        }
    }
    return stockData;
}

//g++ -std=c++17 -Wall -Wextra -I/usr/local/include -I./home/mars/Low-Latency-Trading-Framework web_scraper.cpp ../stock_price.cpp ../util.cpp ../Profiler/performance_profiler.cpp -o web_scraper -lcurl
int main() {
    std::vector<std::string> symbols = {"MSFT", "AMZN", "GOOGL", "META", "NFLX"}; 
    std::string targetDate = "2023-08-02";
    Profiler prof;
    scrape(symbols, targetDate, prof);
    return 0;
}