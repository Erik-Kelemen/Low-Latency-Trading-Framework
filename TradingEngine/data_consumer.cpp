#include <string>
#include <vector>
#include <librdkafka/rdkafkacpp.h>
#include <thread>
#include <algorithm>

#include "../model/stock_price.h"
#include "../Profiler/performance_profiler.h"

/**
 * @class KafkaConsumer
 * @brief A class that represents a Kafka message consumer for stock prices.
 */
class KafkaConsumer {
private:
    std::string brokerAddr_;
    std::string topicName_;
    std::string errstr_;

    RdKafka::Conf* conf_;
    RdKafka::Consumer* consumer_;
    RdKafka::Topic* topic_;
    RdKafka::TopicPartition* partition_;

public:
    /**
     * @brief Constructor to initialize the KafkaConsumer.
     * @param brokerAddr The address of the Kafka broker to connect.
     * @param topicName The name of the topic to consume messages from.
     */
    KafkaConsumer(const std::string& brokerAddr, const std::string& topicName)
        : brokerAddr_(brokerAddr), topicName_(topicName) {
        conf_ = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        conf_->set("bootstrap.servers", brokerAddr_, errstr_);

        consumer_ = RdKafka::Consumer::create(conf_, errstr_);
        if (!consumer_) {
            std::cerr << "Failed to create Kafka consumer: " << errstr_ << std::endl;
        }

        topic_ = RdKafka::Topic::create(consumer_, topicName_, nullptr, errstr_);
        if (!topic_) {
            std::cerr << "Failed to create Kafka topic: " << errstr_ << std::endl;
        }

        partition_ = RdKafka::TopicPartition::create(topicName_, RdKafka::Topic::PARTITION_UA);
    }
    /**
     * @brief Destructor to clean up resources.
     */
    ~KafkaConsumer() {
        delete partition_;
        delete topic_;
        delete consumer_;
        delete conf_;
    }

    /**
     * @brief Function to consume stock price messages from Kafka within the specified lookback period.
     * @param lookbackPeriod The time period (in milliseconds) to look back for stock prices.
     * @return A vector of StockPrice containing the stock prices within the lookback period.
     */
    std::vector<StockPrice> consumeMessages(int lookbackPeriod) {
        std::vector<StockPrice> lookbackWindow;
        int64_t endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        int64_t startTime = endTime - lookbackPeriod;

        RdKafka::ErrorCode err = consumer_->assign({partition_});
        if (err != RdKafka::ERR_NO_ERROR) {
            std::cerr << "Failed to assign partition: " << RdKafka::err2str(err) << std::endl;
            return lookbackWindow;
        }

        RdKafka::Message* msg = nullptr;
        while (true) {
            msg = consumer_->consume(10); // 10ms timeout
            if (msg && msg->err() == RdKafka::ERR_NO_ERROR) {
                int64_t timestamp = msg->timestamp().timestamp;
                if (timestamp >= startTime && timestamp <= endTime) {
                    std::string key = static_cast<const char*>(msg->key()->payload());
                    double price = std::stod(static_cast<const char*>(msg->payload()));
                    lookbackWindow.push_back({key, price});
                }
                delete msg;
            } else if (msg && msg->err() == RdKafka::ERR__PARTITION_EOF) {
                delete msg;
                break;
            } else {
                std::cerr << "Failed to consume message: " << (msg ? msg->errstr() : "null message") << std::endl;
                delete msg;
            }
        }

        consumer_->unassign();
        return lookbackWindow;
    }
};