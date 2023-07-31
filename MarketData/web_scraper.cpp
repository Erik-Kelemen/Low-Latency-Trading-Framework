#include <iostream>
#include <string>
#include <curl/curl.h>
#include <rapidjson/document.h>

const std::string BASE_URL = "https://www.alphavantage.co/query?";
const std::string API_KEY = "YOUR_API_KEY"; //todo

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

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
        // You can extract more data like high, low, close, volume, etc., if needed.
        std::cout << "Date: " << date << ", Open Price: " << openPrice << std::endl;
    }
    profiler.stopComponent("Web Scraper");
}

int main() {
    std::vector<std::string> symbols = {"MSFT", "AMZN", "GOOGL", "META", "NFLX"};

    for (const auto& symbol : symbols) {
        std::string jsonData = fetchStockData(symbol);
        parseStockData(jsonData);
    }

    return 0;
}