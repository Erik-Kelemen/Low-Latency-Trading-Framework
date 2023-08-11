# Low-Latency-Trading-Framework
A C++ simulation for researching the latency impact of different algorithmic trading optimizations, including cache locality, memory management, fast I/O, and parallel processing. The framework includes a market data simulator to interpolate millisecond-level stock prices from real-world S&P 500 company data (extracted using Alpha Vantage) and publishes the results to the trading engine using Kafka.

## Architecture
![alt text](https://github.com/Erik-Kelemen/Low-Latency-Trading-Framework/blob/main/imgs/LLTF.drawio.png)

## MarketData

web_scraper.cpp: Responsible for querying the Exchange API to retrieve real-world historical stock price data at a fine level.  In this framework, Alpha Vantage is used to fetch the data. You can get your own Alpha Vantage API Key for free here: https://www.alphavantage.co/support/#api-key.
The results are then persisted to "exchange_prices.csv".

interpolator.cpp: Handles interpolating gaps in the real-world data at the millisecond level. It reads the prices from "exchange_prices.csv" delivered by the web scraper and, every 10 milliseconds, populates a new entry into a CSV file "interpolated_prices.csv" based on minor random variations (+/- 0.0005 by default).

data_publisher.cpp: Reads "interpolated_prices.csv" and publishes stock prices back to the TradingEngine, controller.cpp, using Apache Kafka.

## TradingEngine

controller.cpp: Accepts stock prices from the Kafka Queue (provided by data_publisher) and organizes the data into a format that can be sent to the trading_strategy.cpp. For persistence purposes and to simulate a real exchange, StockTrades are persisted to a database (in this framework, a local SQLite3 stored in trades.db).

trading_strategy.cpp: Implemented by quant developers using your framework to return trading decisions based on the provided stock prices and the current pool of remaining cash managed by the controller.cpp. The trades returned by trading_strategy.cpp are evaluated to calculate the profits and losses of the trading algorithm given the prices data.

data_receiver.cpp: A consumer for accepting messages from the Kafka Queue and passing prices to the controller.

position_calculator.cpp: An engine for computing the remaining Cash and net P&L given the trades and prices from the controller.

## Performance Profiling

performance_profiler.cpp: Responsible for measuring the latencies of each component in the framework, helping to analyze the efficiency and speed of the system.

## Model

stock_price.cpp: Struct type definition for StockPrice as (ticker, time, price)

stock_trade.cpp: Struct type definition for StockTrade as (ticker, time, qty)

util.cpp: Miscellaneous utility functions for reading/writing from/to streams and files. Primarily used in MarketDatSimulator

## How to Use
I developed this project on a WSL environment, Ubuntu 22.04.2 LTS. The following steps are written for setup required to emulate the project in a similar Linux environment.

### SQLIte3 Setup
1. Install SQLite3 on your system.

```
sudo apt update
sudo apt install sqlite3
sudo apt-get install sqlite3 libsqlite3-dev
sqlite3 --version
```

### Redis Setup:
1. Install Redis

```
sudo apt update
sudo apt install redis-server
```

2. Start Redis Server
    
```
sudo service redis-server start
redis-cli ping
```

3. Using Redis
You can interact with Redis using the redis-cli command. For example, you can set and retrieve values using the following commands:

```
redis-cli set mykey "Hello Redis"
redis-cli get mykey
```

### Apache Kafka Setup:
1. Download Kafka
You can download Kafka from the Apache Kafka website. Open your browser and navigate to the Kafka download page: https://kafka.apache.org/downloads
Download the binary distribution and copy the link to the download URL.

2. Install Kafka

```
wget <paste_the_download_URL_here>
tar -xzf kafka_<version>.tgz
cd kafka_<version>
```

3. Start ZooKeeper
Kafka uses ZooKeeper for managing its cluster. Start ZooKeeper by running the following command:

```
bin/zookeeper-server-start.sh config/zookeeper.properties
```

4. Start Kafka Server
In a new terminal window, navigate to the Kafka directory and start the Kafka server:

```
bin/kafka-server-start.sh config/server.properties
```

5. Create PRICES topic:
Kafka uses topics to organize data streams. In this framework, we need a PRICES topic to pass data from the MarketDataSimulator to the TradingEngine.

```
bin/kafka-topics.sh --create --topic PRICES --bootstrap-server localhost:9092 --partitions 1 --replication-factor 1
```

### Other Steps
1. This project was written and tested in C++17, using G++. If you do not have g++, then you can install and verify it with the following commands
   
```
sudo apt install g++
g++ --version
```

2. You will need the rapidjson library, used in the MarketDataSimulator, for extracting the prices from the Alpha Vantage api calls.
   
```
sudo apt-get install rapidjson-dev
```

Or you can manually install it from the RapidJSON GitHub repository: https://github.com/Tencent/rapidjson
I moved my rapidjson repository to /usr/local/include so it is consistently and easily accessible in my g++ command for compiling and testing individual methods.

3. Replace the alpha vantage key in MarketData/web_scraper.cpp with your own Alpha Vantage API key.


### Trigger 
To compile, run ```make```  and trigger the main executable.

## Future Work
While the current implementation provides a functional low latency trading framework, there are some limitations and areas for potential improvement that could be considered in future iterations:

### Limitations

1. Simulation vs. Real Trading: The current framework is designed for simulation and backtesting purposes. For real trading in live markets, additional considerations such as order execution, partial fills, slippage, and handling of latency spikes need to be incorporated.

2. Risk Management: The framework lacks comprehensive risk management features. Future improvements should include risk controls, position sizing algorithms, and stop-loss mechanisms to protect against adverse market movements.

3. Handling Large Datasets: The current implementation assumes small datasets for simulation. To handle large historical datasets or real-time market data, optimizations like parallel processing and efficient memory management may be required.

4. Robustness and Error Handling: The system should be able to handle unexpected events, API errors, and network failures gracefully. Proper error handling and recovery mechanisms will enhance the system's robustness.

5. Security and Authentication: For real-world applications, security concerns such as authentication, encryption, and secure communication with exchange APIs need to be addressed.

### Improvements
1. Advanced Trading Strategies: Incorporating a wider range of trading strategies (e.g., momentum, mean reversion, statistical arbitrage) will provide users with more options to suit their specific trading goals.

2. Machine Learning Integration: Integrating machine learning models can enhance decision-making processes and optimize trading strategies based on market data patterns and signals.

3. Diversified Asset Support: Expanding the framework to support various asset classes (e.g., equities, futures, options) will enable users to explore trading opportunities across different financial instruments.

4. Configurability and Flexibility: Enhancing configurability through user-defined parameters, strategies, and customizable components will provide more flexibility for different trading approaches.

5. Performance Optimization: To achieve even lower latencies, fine-tuning and optimizing critical components, such as the data retrieval and processing pipeline, can be explored.

6. Monitoring and Analytics: Implementing monitoring tools and performance analytics will allow users to track the system's performance, identify bottlenecks, and make data-driven improvements.

