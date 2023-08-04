#include <iostream>
#include <string>
#include <curl/curl.h>
#include <rapidjson/document.h>

#include "../Profiler/performance_profiler.h"
#include "../stock_price.h" // Include the StockPrice header
#include <vector>

#include <fstream>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);
void scrape(std::vector<std::string> symbols, std::string targetDate);
std::string fetchStockData(const std::string& symbol);
std::vector<StockPrice> parseStockData(const std::string ticker, const std::string& jsonData, const std::string& targetDate);

const std::string BASE_URL = "https://www.alphavantage.co/query?";
const std::string API_KEY = "ALQU3SWWYFF7QHXA"; //replace with your Alpha Vantage API Key
const std::string interval = "1min";

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

void scrape(std::vector<std::string> symbols, std::string targetDate, Profiler profiler){
    profiler.startComponent("Web Scraper");
    std::ofstream outputFile("exchange_prices.csv");
    if (outputFile.is_open()) {
        std::streambuf* originalBuffer = std::cout.rdbuf(); 
        std::cout.rdbuf(outputFile.rdbuf()); 
        
        std::vector<StockPrice> prices;
        
        for (const std::string& symbol : symbols) {
            std::string jsonData = fetchStockData(symbol);
            std::vector<StockPrice> cur_prices = parseStockData(symbol, jsonData, targetDate);
            prices.insert(prices.end(), cur_prices.begin(), cur_prices.end());
        }
        std::cout << "ticker,time,price" << std::endl;
        for(StockPrice p: prices){
            p.print();
        }

        std::cout.rdbuf(originalBuffer);
        
        outputFile.close();
        std::cout << "Data written to file successfully." << std::endl;
    } else {
        std::cerr << "Error opening the file!" << std::endl;
    }
    
    profiler.stopComponent("Web Scraper");
}

std::string fetchStockData(const std::string& symbol) {
    std::string url = BASE_URL + "function=TIME_SERIES_INTRADAY&symbol=" + symbol + "&apikey=" + API_KEY + "&interval=" + interval + "&extended_hours=false&outputsize=full";
    CURL* curl = curl_easy_init();
    std::string response;
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
    
    return response;
}
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

//g++ -std=c++17 -Wall -Wextra -I/home/mars/rapidjson/include -I../Profiler -I../ web_scraper.cpp ../stock_price.cpp ../Profiler/performance_profiler.cpp -o web_scraper -lcurl
int main() {
    std::vector<std::string> symbols = {"MSFT", "AMZN", "GOOGL", "META", "NFLX"}; 
    std::string targetDate = "2023-08-02";
    Profiler prof;
    scrape(symbols, targetDate, prof);
    return 0;
}