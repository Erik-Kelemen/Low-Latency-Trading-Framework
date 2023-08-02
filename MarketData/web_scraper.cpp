#include <iostream>
#include <string>
#include <curl/curl.h>
#include <rapidjson/document.h>
#include "../Profiler/performance_profiler.h"
#include <vector>

class WebScraper {
private:
    const std::string BASE_URL = "https://www.alphavantage.co/query?";
    const std::string API_KEY = "ALQU3SWWYFF7QHXA"; //replace with your Alpha Vantage API Key
    Profiler profiler;

    // static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* response) {
    //     size_t totalSize = size * nmemb;
    //     ((std::string*)response)->append((char*)(contents), totalSize);
    //     return totalSize;
    // }
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
        size_t totalSize = size * nmemb;
        response->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }
public:
    std::string fetchStockData(const std::string& symbol) {
        profiler.startComponent("Web Scraper");
        std::string url = BASE_URL + "function=TIME_SERIES_DAILY&symbol=" + symbol + "&apikey=" + API_KEY;
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
        profiler.stopComponent("Web Scraper");
        std::cout << "Response successful: " << response << std::endl;
        return response;
    }
    void parseStockData(const std::string& jsonData) {
        profiler.startComponent("Web Scraper");
        rapidjson::Document document;
        document.Parse(jsonData.c_str());

        if (document.HasParseError()) {
            std::cerr << "JSON parsing error!" << std::endl;
            return;
        }

        const rapidjson::Value& timeSeries = document["Time Series (Daily)"];

        for (auto it = timeSeries.MemberBegin(); it != timeSeries.MemberEnd(); ++it) {
            const std::string& date = it->name.GetString();
            const double openPrice = std::stod(it->value["1. open"].GetString());
            std::cout << "Date: " << date << ", Open Price: " << openPrice << std::endl;
        }
        profiler.stopComponent("Web Scraper");
    }
};

//g++ -std=c++17 -Wall -Wextra -I/home/mars/rapidjson/include web_scraper.cpp -o web_scraper
//g++ -std=c++17 -Wall -Wextra -I/home/mars/rapidjson/include -I../Profiler web_scraper.cpp ../Profiler/performance_profiler.cpp -o web_scraper -lcurl

int main() {
    std::vector<std::string> symbols = {"MSFT", "AMZN", "GOOGL", "META", "NFLX"}; 
    WebScraper scraper;
    for (const auto& symbol : symbols) {
        std::string jsonData = scraper.fetchStockData(symbol);
        // scraper.parseStockData(jsonData);
    }

    return 0;
}