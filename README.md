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

## Future Work
While the current implementation provides a functional low latency trading framework, there are some limitations and areas for potential improvement that could be considered in future iterations:

### Limitations

Simulation vs. Real Trading: The current framework is designed for simulation and backtesting purposes. For real trading in live markets, additional considerations such as order execution, partial fills, slippage, and handling of latency spikes need to be incorporated.

Risk Management: The framework lacks comprehensive risk management features. Future improvements should include risk controls, position sizing algorithms, and stop-loss mechanisms to protect against adverse market movements.

Handling Large Datasets: The current implementation assumes small datasets for simulation. To handle large historical datasets or real-time market data, optimizations like parallel processing and efficient memory management may be required.

Robustness and Error Handling: The system should be able to handle unexpected events, API errors, and network failures gracefully. Proper error handling and recovery mechanisms will enhance the system's robustness.

Security and Authentication: For real-world applications, security concerns such as authentication, encryption, and secure communication with exchange APIs need to be addressed.

### Improvements
Advanced Trading Strategies: Incorporating a wider range of trading strategies (e.g., momentum, mean reversion, statistical arbitrage) will provide users with more options to suit their specific trading goals.

Machine Learning Integration: Integrating machine learning models can enhance decision-making processes and optimize trading strategies based on market data patterns and signals.

Diversified Asset Support: Expanding the framework to support various asset classes (e.g., equities, futures, options) will enable users to explore trading opportunities across different financial instruments.

Configurability and Flexibility: Enhancing configurability through user-defined parameters, strategies, and customizable components will provide more flexibility for different trading approaches.

Performance Optimization: To achieve even lower latencies, fine-tuning and optimizing critical components, such as the data retrieval and processing pipeline, can be explored.

Monitoring and Analytics: Implementing monitoring tools and performance analytics will allow users to track the system's performance, identify bottlenecks, and make data-driven improvements.

Documentation and Tutorials: Improving documentation and providing comprehensive tutorials will help users understand the framework better and facilitate its adoption.
