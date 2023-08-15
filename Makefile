CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I$(HOME)/Low-Latency-Trading-Framework -I/usr/include -I/usr/local/include/cpp_redis/includes -I/usr/local/include/tacopie/includes

SRC_DIR := $(HOME)/Low-Latency-Trading-Framework
MARKETDATA_DIR := $(SRC_DIR)/MarketData
TRADINGENGINE_DIR := $(SRC_DIR)/TradingEngine
PROFILER_DIR := $(SRC_DIR)/Profiler
MODEL_DIR := $(SRC_DIR)/Model

LIBS := -lcurl -lsqlite3 -lcpp_redis -lrapidjson

MAIN_SRCS := $(SRC_DIR)/main.cpp
MARKETDATA_SRCS := $(wildcard $(MARKETDATA_DIR)/*.cpp)
TRADINGENGINE_SRCS := $(wildcard $(TRADINGENGINE_DIR)/*.cpp)
PROFILER_SRCS := $(wildcard $(PROFILER_DIR)/*.cpp)
MODEL_SRCS := $(wildcard $(MODEL_DIR)/*.cpp)

MAIN_OBJS := $(MAIN_SRCS:.cpp=.o)
MARKETDATA_OBJS := $(MARKETDATA_SRCS:.cpp=.o)
TRADINGENGINE_OBJS := $(TRADINGENGINE_SRCS:.cpp=.o)
PROFILER_OBJS := $(PROFILER_SRCS:.cpp=.o)
MODEL_OBJS := $(MODEL_SRCS:.cpp=.o)

TARGET := LowLatencyTradingFramework

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(MAIN_OBJS) $(MARKETDATA_OBJS) $(TRADINGENGINE_OBJS) $(PROFILER_OBJS) $(MODEL_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(MAIN_OBJS) $(MARKETDATA_OBJS) $(TRADINGENGINE_OBJS) $(PROFILER_OBJS) $(MODEL_OBJS)
