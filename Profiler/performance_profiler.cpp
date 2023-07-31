#include <iostream>
#include <chrono>
#include <string>
#include <unordered_map>

class Profiler {
private:
    std::unordered_map<std::string, double> componentTimes_;
    std::chrono::steady_clock::time_point startTime_;

public:
    Profiler() {
        startTime_ = std::chrono::steady_clock::now();
    }

    void startComponent(const std::string& componentName) {
        componentTimes_[componentName] = 0.0;
        componentTimes_[componentName] -= getCurrentTime();
    }

    void stopComponent(const std::string& componentName) {
        componentTimes_[componentName] += getCurrentTime();
    }

    double getTotalTime() const {
        return getCurrentTime();
    }

    void printComponentTimes() const {
        std::cout << "Component times:" << std::endl;
        for (const auto& entry : componentTimes_) {
            std::cout << entry.first << ": " << entry.second << " seconds" << std::endl;
        }
    }

private:
    double getCurrentTime() const {
        auto currentTime = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(currentTime - startTime_).count();
    }
};
