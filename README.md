# Low-Latency-Trading-Framework
A C++ simulation for researching the latency impact of different algorithmic trading optimizations

## Architecture
![alt text](https://github.com/Erik-Kelemen/Low-Latency-Trading-Framework/blob/main/imgs/LLFT-Architecture.drawio.png)

## MarketData Component:

web_scraper.cpp: Responsible for querying the Exchange API to retrieve real-world historical stock price data at a fine level.

interpolator.cpp: Handles interpolating gaps in the real-world data at the millisecond level.

data_publisher.cpp: Publishes real-world stock prices to the next component, the TradingEngine.
## TradingEngine:

central controller.cpp: Accepts stock prices from the Kafka Queue (provided by data_publisher) and organizes the data into a format that can be sent to the trading_strategy.cpp.
trading_strategy.cpp: Implemented by quant developers using your framework to return trading decisions based on the provided stock prices and the current pool of remaining cash managed by the controller.cpp.
## Profit & Loss Calculation:

The trades returned by trading_strategy.cpp are evaluated to calculate the profits and losses of the trading algorithm given the real-world data.
## Performance Profiling:

performance_profiler.cpp: Responsible for measuring the latencies of each component in the framework, helping to analyze the efficiency and speed of the system.
