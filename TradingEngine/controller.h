#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <vector>

struct StockPrice {
    std::string symbol;
    double price;
};

class KafkaConsumer {
private:
    std::string brokerAddr_;
    std::string topicName_;
    std::string errstr_;

    class Conf* conf_;
    class Consumer* consumer_;
    class Topic* topic_;
    class TopicPartition* partition_;

public:
    // Constructor takes the broker address and topic name
    KafkaConsumer(const std::string& brokerAddr, const std::string& topicName);

    // Destructor to clean up resources
    ~KafkaConsumer();

    // Function to consume stock price messages from Kafka for a given lookback period
    std::vector<StockPrice> consumeMessages(int lookbackPeriod);
};
class Controller { 

public:
    void runTradingFramework();
}
#endif
