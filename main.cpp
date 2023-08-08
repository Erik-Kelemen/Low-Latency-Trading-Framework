#include "TradingEngine/controller.cpp"
#include "Profiler/performance_profiler.h"
#include <vector>
#include <string>
int main() {
    std::vector<std::string> symbols = {"MSFT", "AMZN", "GOOGL", "META", "NFLX"};
    const std::string date = "2023-08-02";
    Profiler profiler;
    Controller controller(symbols, date, profiler);
    controller.runTradingFramework();
    profiler.printComponentTimes();
    return 0;
}
