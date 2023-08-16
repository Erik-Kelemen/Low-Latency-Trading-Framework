#include "util.h"
#include "stock_price.h"
#include <sstream>
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

std::vector<StockPrice> read(const std::string sourceFile) {
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

void write(const std::string header, const std::string destination, std::vector<StockPrice> prices) {
    std::ofstream outputFile(destination);

    if (outputFile.is_open()) {
        std::streambuf* originalBuffer = std::cout.rdbuf(); 
        std::cout.rdbuf(outputFile.rdbuf()); 
        
        std::cout << header << std::endl;
        
        for(StockPrice p: prices)
            p.print();

        std::cout.rdbuf(originalBuffer);
        outputFile.close();
    } else {
        std::cerr << "Error opening the file: " << destination << std::endl;
    }
}
