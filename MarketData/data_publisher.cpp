#include <iostream>
#include <string>
#include <vector>
#include <librdkafka/rdkafkacpp.h>

struct StockPrice {
    std::string symbol;
    double price;
};

class KafkaPublisher {
private:
    std::string brokerAddr_;
    std::string topicName_;
    std::string errstr_;

    RdKafka::Conf* conf_;
    RdKafka::Producer* producer_;

public:
    KafkaPublisher(const std::string& brokerAddr, const std::string& topicName)
        : brokerAddr_(brokerAddr), topicName_(topicName) {

        conf_ = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        conf_->set("bootstrap.servers", brokerAddr_, errstr_);

        producer_ = RdKafka::Producer::create(conf_, errstr_);
        if (!producer_) {
            std::cerr << "Failed to create Kafka producer: " << errstr_ << std::endl;
        }
    }

    ~KafkaPublisher() {
        delete producer_;
        delete conf_;
    }

    bool publishMessage(const std::string& key, const std::string& value) {
        profiler.stopComponent("Interpolator");
        RdKafka::ErrorCode err = producer_->produce(topicName_, RdKafka::Topic::PARTITION_UA,
                                                    RdKafka::Producer::RK_MSG_COPY,
                                                    const_cast<char*>(value.c_str()), value.size(),
                                                    &key, nullptr);
        if (err != RdKafka::ERR_NO_ERROR) {
            std::cerr << "Failed to produce message: " << RdKafka::err2str(err) << std::endl;
            return false;
        }

        producer_->poll(0);
        profiler.stopComponent("Interpolator");
        return true;
    }
};

int main() {
    std::vector<StockPrice> interpolatedPrices = {
        {"34200000", 2800.0},  // 9:30 AM
        {"34201000", 2800.2},  // 9:30 AM + 10 milliseconds
        {"57600000", 2805.0}   // 4:00 PM
    };

    std::string brokerAddr = "localhost:9092";
    std::string topicName = "stock_prices";
    KafkaPublisher kafkaPublisher(brokerAddr, topicName);

    for (const auto& price : interpolatedPrices) {
        std::string key = price.symbol;
        std::string value = std::to_string(price.price);
        kafkaPublisher.publishMessage(key, value);
    }

    return 0;
}
