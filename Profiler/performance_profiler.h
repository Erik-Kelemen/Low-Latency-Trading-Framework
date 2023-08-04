#pragma once

#ifndef PROFILER_H
#define PROFILER_H

#include <iostream>
#include <chrono>
#include <string>
#include <unordered_map>

class Profiler {
private:
    std::unordered_map<std::string, double> componentTimes_;
    std::chrono::steady_clock::time_point startTime_;

public:
    Profiler();

    void startComponent(const std::string& componentName);
    void stopComponent(const std::string& componentName);
    double getTotalTime() const;
    void printComponentTimes() const;

private:
    double getCurrentTime() const;
};

#endif 
