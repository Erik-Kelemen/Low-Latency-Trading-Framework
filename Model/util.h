#pragma once

#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>
#include <fstream>
#include "stock_price.h"

//File persistence
const std::string exchangeFile = "exchange_prices.csv";
const std::string interpolatedFile = "interpolated_prices.csv";

//Kafka
const std::string brokerAddr = "localhost:9092";
const std::string topicName = "PRICES";

/*
 * Splits a string into a vector of substrings based on the delimiter ','.
 * @param input The input string to split.
 * @return A vector of substrings obtained by splitting the input string.
 */
std::vector<std::string> splitStringByComma(const std::string& input);

/*
 * Reads stock data from a CSV file and returns a vector of StockPrice objects.
 * @param sourceFile The path to the CSV file containing the stock data.
 * @return A vector of StockPrice objects representing the stock data read from the file.
 */
std::vector<StockPrice> read(const std::string sourceFile);

/*
 * Writes stock data to a CSV file.
 * @param destination The path to the output CSV file.
 * @param prices A vector of StockPrice objects to be written to the file.
 * @param header The CSV header to be written at the top of the file. Default value is "ticker,time,price".
 */
void write(const std::string header, const std::string targetFile, std::vector<StockPrice> prices);


#endif // UTIL_H
