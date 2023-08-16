#include <string>
#include <vector>
#include <librdkafka/rdkafkacpp.h>

#include "../Profiler/performance_profiler.h"
#include "../Model/stock_price.h"
#include "../Model/util.h"

class KafkaPublisher {
private:
    std::string errstr;

    RdKafka::Conf* conf;
    RdKafka::Producer* producer;
    Profiler& profiler;
public:
    KafkaPublisher(Profiler& profiler)
        : profiler(profiler) {
        profiler.startComponent("Data Publisher");
        conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        conf->set("bootstrap.servers", brokerAddr, errstr);

        producer = RdKafka::Producer::create(conf, errstr);
        if (!producer) 
            std::cerr << "Failed to create Kafka producer: " << errstr << std::endl;
        
        profiler.stopComponent("Data Publisher");
    }

    ~KafkaPublisher() {
        delete producer;
        delete conf;
    }

    void publish(const std::vector<StockPrice>& prices) {
        profiler.startComponent("Data Publisher");
        
        for (const StockPrice& price : prices) {
            std::string key = price.ticker;
            int64_t timestamp = stoi(price.time);
            std::string value = std::to_string(price.price);
            publishMessage(timestamp, key, value);
        }

        profiler.stopComponent("Data Publisher");
    }
    
    void publish_from_file(){
        publish(read(interpolatedFile));
    }

    bool publishMessage(int64_t timestamp, const std::string& key, const std::string& value) {
        RdKafka::ErrorCode err = producer->produce(topicName, RdKafka::Topic::PARTITION_UA,
                                                    RdKafka::Producer::RK_MSG_COPY,
                                                    const_cast<char*>(value.c_str()), value.size(),
                                                    const_cast<char*>(key.c_str()), key.size(),
                                                    timestamp,
                                                    nullptr);
        if (err != RdKafka::ERR_NO_ERROR) {
            std::cerr << "Failed to produce message: " << RdKafka::err2str(err) << std::endl;
            return false;
        }

        producer->poll(0);
        return true;
    }
};
