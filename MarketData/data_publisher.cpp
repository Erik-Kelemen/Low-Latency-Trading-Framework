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

class KafkaPublisher {
private:
    std::string brokerAddr_;
    std::string topicName_;
    std::string errstr_;

    RdKafka::Conf* conf_;
    RdKafka::Producer* producer_;
    Profiler profiler;
public:
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

    ~KafkaPublisher() {
        delete producer_;
        delete conf_;
    }
    void publish(){
        this->profiler.startComponent("Data Publisher");
        std::vector<StockPrice> prices = read(interpolatedFile);
        for (const StockPrice& price : prices) {
            std::string key = price.ticker;
            std::string value = std::to_string(price.price);
            this->publishMessage(key, value);
        }
        this->profiler.stopComponent("Data Publisher");
    }
    bool publishMessage(const std::string& key, const std::string& value) {
        profiler.startComponent("Interpolator");
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
    std::string brokerAddr = "localhost:9092";
    std::string topicName = "prices";
    Profiler prof;
    KafkaPublisher kafkaPublisher(brokerAddr, topicName, prof);

    return 0;
}
