#include <string>
#include <vector>
#include <librdkafka/rdkafkacpp.h>
#include <thread>
#include <algorithm>

#include "../Model/stock_price.h"
#include "../Profiler/performance_profiler.h"

/**
 * @class KafkaConsumer
 * @brief A class that represents a Kafka message consumer for stock prices.
 */
class KafkaConsumer {
private:
    std::string brokerAddr;
    std::string topicName;
    std::string errstr;

    RdKafka::Conf* conf;
    RdKafka::Consumer* consumer;
    RdKafka::Topic* topic;
    const uint32_t partition = RdKafka::Topic::PARTITION_UA;
    Profiler& profiler;
public:
    /**
     * @brief Constructor to initialize the KafkaConsumer.
     * @param brokerAddr The address of the Kafka broker to connect.
     * @param topicName The name of the topic to consume messages from.
     */
    KafkaConsumer(Profiler& profiler)
        : profiler(profiler) {
        this->profiler.startComponent("Data Consumer");
        conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        conf->set("bootstrap.servers", brokerAddr, errstr);

        consumer = RdKafka::Consumer::create(conf, errstr);
        if (!consumer) {
            std::cerr << "Failed to create Kafka consumer: " << errstr << std::endl;
        }

        topic = RdKafka::Topic::create(consumer, topicName, nullptr, errstr);
        if (!topic) {
            std::cerr << "Failed to create Kafka topic: " << errstr << std::endl;
        }
        this->profiler.stopComponent("Data Consumer");
    }
    /**
     * @brief Destructor to clean up resources.
     */
    ~KafkaConsumer() {
        delete topic;
        delete consumer;
        delete conf;
    }

    /**
     * @brief Function to consume stock price messages from Kafka within the specified lookback period.
     * @param lookbackPeriod The time period (in milliseconds) to look back for stock prices.
     * @return A vector of StockPrice containing the stock prices within the lookback period.
     */
    std::vector<StockPrice> consumeMessages(int lookbackPeriod) {
        this->profiler.startComponent("Data Consumer");
        std::vector<StockPrice> lookbackWindow;
        int64_t endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        int64_t startTime = endTime - lookbackPeriod;
        RdKafka::ErrorCode err = consumer->start(topic, partition, (int64_t)0);
        if (err != RdKafka::ERR_NO_ERROR) {
            std::cerr << "Failed to assign partition: " << RdKafka::err2str(err) << std::endl;
            return lookbackWindow;
        }

        RdKafka::Message* msg = nullptr;
        while (true) {
            msg = consumer->consume(topic, partition, 10); // 10ms timeout
            if (msg && msg->err() == RdKafka::ERR_NO_ERROR) {
                int64_t timestamp = msg->timestamp().timestamp;
                if (timestamp >= startTime && timestamp <= endTime) {
                    std::string key = reinterpret_cast<const char*>(msg->key());
                    double price = std::stod(static_cast<const char*>(msg->payload()));
                    lookbackWindow.push_back({key, std::to_string(timestamp), price});
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

        consumer->stop(topic, partition);
        this->profiler.stopComponent("Data Consumer");
        return lookbackWindow;
    }
};
