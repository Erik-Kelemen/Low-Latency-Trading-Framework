#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>
#include <fstream>
#include "stock_price.h" // Include the StockPrice header

std::vector<std::string> splitStringByComma(const std::string& input);
std::vector<StockPrice> read(const std::string& sourceFile);

#endif // UTIL_H
