#include <iostream>
#include <string>
#include <vector>
#include <librdkafka/rdkafkacpp.h>

#include "../util.h"
#include "../Profiler/performance_profiler.h"
#include "../stock_price.h"
#include <fstream>
#include <sstream>

const std::string& interpolatedFile = "interpolated_prices.csv";

/**
 * @class KafkaPublisher
 * @brief A class that represents a Kafka message publisher for stock prices.
 */
class KafkaPublisher {
private:
    std::string brokerAddr_;
    std::string topicName_;
    std::string errstr_;

    RdKafka::Conf* conf_;
    RdKafka::Producer* producer_;
    Profiler profiler;
public:
    /**
     * @brief Constructor to initialize the KafkaPublisher.
     * @param brokerAddr The address of the Kafka broker to connect.
     * @param topicName The name of the topic to which messages will be published.
     * @param profiler The Profiler object for performance tracking.
     */
    KafkaPublisher(const std::string& brokerAddr, const std::string& topicName, Profiler profiler)
        : brokerAddr_(brokerAddr), topicName_(topicName) {
        this->profiler = profiler;
        this->profiler.startComponent("Data Publisher");
        conf_ = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        conf_->set("bootstrap.servers", brokerAddr_, errstr_);

        producer_ = RdKafka::Producer::create(conf_, errstr_);
        if (!producer_) {
            std::cerr << "Failed to create Kafka producer: " << errstr_ << std::endl;
        }
        this->profiler.stopComponent("Data Publisher");
    }

    /**
     * @brief Destructor to clean up resources.
     */
    ~KafkaPublisher() {
        delete producer_;
        delete conf_;
    }

    /**
     * @brief Function to publish stock prices to the Kafka topic.
     */
    void publish(){
        this->profiler.startComponent("Data Publisher");
        std::vector<StockPrice> prices = read(interpolatedFile);
        for (const StockPrice& price : prices) {
            std::string key = price.ticker;
            int64_t timestamp = stoi(price.time);
            std::string value = std::to_string(price.price);
            this->publishMessage(timestamp, key, value);
        }
        this->profiler.stopComponent("Data Publisher");
    }

    /**
     * @brief Function to publish a single message to the Kafka topic.
     * @param timestamp The timestamp for the message.
     * @param key The message key.
     * @param value The message value.
     * @return `true` if the message was successfully published, `false` otherwise.
     */
    bool publishMessage(int64_t timestamp, const std::string& key, const std::string& value) {
        RdKafka::ErrorCode err = producer_->produce(topicName_, RdKafka::Topic::PARTITION_UA,
                                                    RdKafka::Producer::RK_MSG_COPY,
                                                    const_cast<char*>(value.c_str()), value.size(),
                                                    const_cast<char*>(key.c_str()), key.size(),
                                                    timestamp,
                                                    nullptr);
        if (err != RdKafka::ERR_NO_ERROR) {
            std::cerr << "Failed to produce message: " << RdKafka::err2str(err) << std::endl;
            return false;
        }

        producer_->poll(0);
        return true;
    }
};

//g++ -std=c++17 -Wall -Wextra -I./home/mars/Low-Latency-Trading-Framework data_publisher.cpp ../stock_price.cpp ../util.cpp ../Profiler/performance_profiler.cpp -o data_publisher -L./home/mars/Low-Latency-Trading-Framework/kafka_2.13-3.5.1 -lrdkafka++ -lrdkafka -lz -lpthread

int main() {
    std::string brokerAddr = "localhost:9092";
    std::string topicName = "PRICES";
    Profiler prof;
    KafkaPublisher kafkaPublisher(brokerAddr, topicName, prof);

    return 0;
}
