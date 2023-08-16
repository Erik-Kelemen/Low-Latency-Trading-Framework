#include "TradingEngine/controller.cpp"
#include "TradingEngine/trading_engine.h"
#include "Profiler/performance_profiler.h"
#include <vector>
#include <string>
int main() {
    
    std::vector<std::string> symbols = {"MSFT", "AMZN", "GOOGL", "META", "NFLX"};
    std::vector<std::string> dates = {"2023-08-02"};
    double cash = 1000000.0;
    int lookbackPeriod = 30000;
    Profiler profiler;
    TradingEngine tradingEngine(profiler);
    Controller controller(tradingEngine, cash, lookbackPeriod, symbols, dates, profiler);
    controller.runTradingFramework();
    profiler.printComponentTimes();
    return 0;
}
