#include "performance_profiler.h"

Profiler::Profiler() {
    startTime_ = std::chrono::steady_clock::now();
}

void Profiler::startComponent(const std::string& componentName) {
    componentTimes_[componentName] = 0.0;
    componentTimes_[componentName] -= getCurrentTime();
}

void Profiler::stopComponent(const std::string& componentName) {
    componentTimes_[componentName] += getCurrentTime();
}

double Profiler::getTotalTime() const {
    return getCurrentTime();
}

void Profiler::printComponentTimes() const {
    std::cout << "Component times:" << std::endl;
    for (const auto& entry : componentTimes_) {
        std::cout << entry.first << ": " << entry.second << " seconds" << std::endl;
    }
}

double Profiler::getCurrentTime() const {
    auto currentTime = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(currentTime - startTime_).count();
}
