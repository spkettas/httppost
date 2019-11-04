#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <sys/time.h>
#include <getopt.h>
#include <unistd.h>
#include "rdkafkacpp.h"


static bool exit_eof = true;
static int eof_cnt = 0;
static int partition_cnt = 0;
static long msg_cnt = 0;
static int64_t msg_bytes = 0;


///
class ExampleEventCb : public RdKafka::EventCb
{
public:
    void event_cb(RdKafka::Event& event)
    {
        switch (event.type())
        {
            case RdKafka::Event::EVENT_ERROR:
                std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " << event.str() << std::endl;
                if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
                {
                    break;
                }

            case RdKafka::Event::EVENT_STATS:
                std::cerr << "\"STATS\": " << event.str() << std::endl;
                break;

            case RdKafka::Event::EVENT_LOG:
                fprintf(stderr, "LOG-%i-%s: %s\n", event.severity(), event.fac().c_str(), event.str().c_str());
                break;

            case RdKafka::Event::EVENT_THROTTLE:
                std::cerr << "THROTTLED: " << event.throttle_time() << "ms by " << event.broker_name() << " id "
                          << (int) event.broker_id() << std::endl;
                break;

            default:
                std::cerr << "EVENT " << event.type() << " (" << RdKafka::err2str(event.err()) << "): " << event.str()
                          << std::endl;
                break;
        }
    }
};


///
void msg_consume(RdKafka::Message* message, void* opaque)
{
    switch (message->err())
    {
        case RdKafka::ERR__TIMED_OUT:
            //std::cerr << "RdKafka::ERR__TIMED_OUT"<<std::endl;
            break;

        case RdKafka::ERR_NO_ERROR:
            fprintf(stderr, "%.*s\n", (message->len()), (message->payload()));
            break;
        case RdKafka::ERR__PARTITION_EOF:
            /* Last message */
            if (exit_eof && ++eof_cnt == partition_cnt)
            {
                std::cerr << "%% EOF reached for all " << partition_cnt << " partition(s)" << std::endl;
            }
            break;

        case RdKafka::ERR__UNKNOWN_TOPIC:
        case RdKafka::ERR__UNKNOWN_PARTITION:
            std::cerr << "Consume failed: " << message->errstr() << std::endl;
            break;

        default:
            /* Errors */
            std::cerr << "Consume failed: " << message->errstr() << std::endl;
    }
}


///
class ExampleConsumeCb : public RdKafka::ConsumeCb
{
public:
    void consume_cb(RdKafka::Message& msg, void* opaque)
    {
        msg_consume(&msg, opaque);
    }
};


/// main
int main(int argc, char** argv)
{
    std::string brokers = "172.21.0.5:9092";
    std::string topic_str = "test";

    if (argc < 3)
    {
        printf("%s: broker topic\n", basename(argv[0]));
        return 1;
    }

    brokers = argv[1];
    topic_str = argv[2];

    std::vector<std::string> topics;
    std::string group_id = "101";
    std::string errstr;

    topics.push_back(topic_str);

    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    RdKafka::Conf* tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    // group.id必须设置
    if (conf->set("group.id", group_id, errstr) != RdKafka::Conf::CONF_OK)
    {
        std::cerr << errstr << std::endl;
        exit(1);
    }

    // Conf
    conf->set("metadata.broker.list", brokers, errstr);

    ExampleConsumeCb ex_consume_cb;
    conf->set("consume_cb", &ex_consume_cb, errstr);

    ExampleEventCb ex_event_cb;
    conf->set("event_cb", &ex_event_cb, errstr);
    conf->set("default_topic_conf", tconf, errstr);

    // 创建消费者
    RdKafka::KafkaConsumer* consumer = RdKafka::KafkaConsumer::create(conf, errstr);
    if (!consumer)
    {
        std::cerr << "Failed to create consumer: " << errstr << std::endl;
        exit(1);
    }
    std::cout << "% Created consumer " << consumer->name() << std::endl;

    // 订阅消息
    RdKafka::ErrorCode err = consumer->subscribe(topics);
    if (err)
    {
        std::cerr << "Failed to subscribe to " << topics.size() << " topics: " << RdKafka::err2str(err) << std::endl;
        exit(1);
    }

    while (1)
    {
        // 5000毫秒未订阅到消息，触发RdKafka::ERR__TIMED_OUT
        RdKafka::Message* msg = consumer->consume(5000);
        msg_consume(msg, NULL);
        consumer->poll(0);
        delete msg;
    }

    consumer->close();
    delete consumer;
    delete conf;
    delete tconf;

    std::cerr << "% Consumed " << msg_cnt << " messages (" << msg_bytes << " bytes)" << std::endl;

    // wait
    RdKafka::wait_destroyed(5000);
    return 0;
}
