#include "util.h"
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

std::vector<StockPrice> read(const std::string& sourceFile) {
    std::vector<StockPrice> rows;
    std::ifstream file(sourceFile);
    std::string line;
    std::vector<std::string> lines;
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << sourceFile << std::endl;
        return rows;
    }

    std::getline(file, line); //skip column headers

    while (std::getline(file, line)) {
        lines = splitStringByComma(line);
        std::string ticker = lines[0], time = lines[1];
        double price = std::stod(lines[2]);
        rows.push_back({ticker, time, price});
    }
    file.close();
    return rows;
}
