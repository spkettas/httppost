#include <unistd.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <getopt.h>
#include "rdkafkacpp.h"


///
class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb
{
public:
    void dr_cb(RdKafka::Message& message)
    {
        std::cout << "Message delivery for (" << message.len() << " bytes): " << message.errstr() << std::endl;
        if (message.key())
        {
            std::cout << "Key: " << *(message.key()) << ";" << std::endl;
        }
    }
};

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

            default:
                std::cerr << "EVENT " << event.type() << " (" << RdKafka::err2str(event.err()) << "): " << event.str()
                          << std::endl;
                break;
        }
    }
};


/// main
int main(int argc, char **argv)
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

    std::string errstr;
    std::string line = "hello world msg";

    int32_t partition = RdKafka::Topic::PARTITION_UA;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    RdKafka::Conf* tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    // Conf
    conf->set("metadata.broker.list", brokers, errstr);
    conf->set("default_topic_conf", tconf, errstr);

    ExampleEventCb ex_event_cb;
    //conf->set("event_cb", &ex_event_cb, errstr);

    ExampleDeliveryReportCb ex_dr_cb;
    //conf->set("dr_cb", &ex_dr_cb, errstr);

    // 创建生产者
    RdKafka::Producer* producer = RdKafka::Producer::create(conf, errstr);
    if (!producer)
    {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
        exit(1);
    }
    std::cout << "% Created producer " << producer->name() << std::endl;

    // 发送消息
    for (auto i = 0; i < 5; i++)
    {
        RdKafka::ErrorCode resp = producer->produce(topic_str, partition,
                RdKafka::Producer::RK_MSG_COPY,
                const_cast<char*>(line.c_str()), line.size(),
                NULL, 0,
                0, NULL);
        if (resp != RdKafka::ERR_NO_ERROR)
        {
            std::cerr << "% Produce failed: " << RdKafka::err2str(resp) << std::endl;
        }
        else
        {
            std::cerr << "% Produced message (" << line.size() << " bytes)" << std::endl;
        }

        producer->poll(0);
    }

    // produce为异步发送，在删除实例前可等待ns
    sleep(2);
    delete producer;
    delete conf;
    delete tconf;

    // wait
    RdKafka::wait_destroyed(5000);
    return 0;
}
