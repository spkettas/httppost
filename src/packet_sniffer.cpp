#include "packet_sniffer.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <string.h>
#include "sysconfig.h"

// kafka
#include "librdkafka/rdkafkacpp.h"
// libpcap
#include <pcap.h>


using namespace DebugUtil;

// http头部解析
extern bool ParseHttp(char* data, int len, URLInfo* info);


///
PacketSniffer::PacketSniffer(const std::string& dev, SysLog* Log)
        : Log(Log)
          , m_producer(nullptr)
          , m_Mempool(CSysConfig::Inst()->poolsize)
          , flowtable(1000000)
{
    strncpy(netdev, dev.c_str(), dev.size());
    netdev[dev.size()] = '\0';
}


PacketSniffer::~PacketSniffer()
{
    if (m_producer)
    {
        delete m_producer;
        m_producer = nullptr;
    }
}


bool PacketSniffer::Start()
{
    // 初始化kafka
    InitKafka();

    // 用libpcap采集
    Log->PrintLog(INFO_LEVEL, "Init libpcap engine");
    PcapSniffer(netdev);

    return true;
}


void PacketSniffer::HandleFrame(char* pdata)
{
    if (pdata == NULL)
    {
        return;
    }

    struct ethhdr* pe;
    struct iphdr* iphead;
    struct tcphdr* tcp;

    char* Data = NULL;
    unsigned int Length = 0;

    URLInfo headInfo = {};
    int offset = 0;

    pe = (struct ethhdr*) pdata;

    /// vlan
    if (ntohs(pe->h_proto) == ETHERTYPE_VLAN)       // vlan
    {
        offset = 4;
    }
    else if (ntohs(pe->h_proto) != ETHERTYPE_IP)    // ip
    {
        return;
    }

    /// ip
    iphead = (struct iphdr*) (pdata + offset + sizeof(struct ethhdr));
    if (NULL == iphead)
    {
        return;
    }

    if (iphead->protocol != IPPROTO_TCP)
    {
        return;
    }

    /// tcp
    tcp = (struct tcphdr*) ((char*) iphead + iphead->ihl * 4);
    if (NULL == tcp)
    {
        return;
    }

    // 80
    if (ntohs(tcp->dest) != CSysConfig::Inst()->port)
    {
        return;
    }

    Length = htons(iphead->tot_len) - iphead->ihl * 4 - tcp->doff * 4;
    if (Length < 20 || Length > 3000)
    {
        return;
    }

    // 获取tcp数据部分，由于很多包带有option，不能用sizeof(tcphdr)
    //Data = (char*) tcp + sizeof(struct tcphdr);
    Data = (char*) tcp + tcp->doff * 4;

    // 过滤GET请求
    if (ntohl(*(unsigned int*) Data) == VALUE_GET)
    {
        return;
    }

    // 解析post请求
    if (!ParseHttp(Data, Length, &headInfo)
            || (headInfo.contentType == TYPE_APPLICATION) && headInfo.contentLen > 0)
    {
        Log->PrintLog(DEBUG_LEVEL, "Port:%d Parse error and contype-type:%d",
                ntohs(tcp->source), headInfo.contentType);
        return;
    }

    CREATE_FLOWKEY(iphead, tcp);

    // &&&
    Log->PrintLog(DEBUG_LEVEL, "Port:%s Contentlen:%d Len:%d content:\n%s", key.c_str(), headInfo.contentLen, Length,
            Data);

    // 流表是否存在该流
    auto it = flowtable.find(key);
    if (it == flowtable.end())
    {
        Log->PrintLog(DEBUG_LEVEL, "111111:Port:%s", key.c_str());  // &&&

        // 没有上下文的包，直接返回
        if (headInfo.contentType == TYPE_UNKNOWN)
        {
            // 修正有些错乱的包，先发body后发header
            RecvModify(Data, Length);
            return;
        }
        else if (headInfo.contentLen >= MAX_BUF_LEN)
        {
            Log->PrintLog(ERROR_LEVEL, "Packet too large: %lu", headInfo.contentLen);
            return;
        }
        Log->PrintLog(DEBUG_LEVEL, "222222:Port:%s", key.c_str());  // &&&

        // 获取节点
        mutex.lock();
        CReqCache* cache = m_Mempool.GetNode();
        mutex.unlock();

        if (cache == nullptr)
        {
            Log->PrintLog(ERROR_LEVEL, "Pool used out and need to free");
            return;
        }

        cache->cLen = headInfo.contentLen;
        cache->usedCnt = 0;
        cache->data[0] = '\0';

        // 判断头部是否与内容粘住
        this->RecvStart(cache, Data, Length);

        Log->PrintLog(DEBUG_LEVEL, "===New:Port:%d Contentlen:%d UsedCnt:%d", ntohs(tcp->source), cache->cLen,
                cache->usedCnt);  // &&&

        // 添加流表与lru
        mutex.lock();
        flowtable.insert({ key, cache });
        lrukey.emplace_back(key);
        mutex.unlock();
    }
    else
    {
        auto cache = it->second;
        size_t sliceCnt = Length;   // 当前切片长度

        if (cache->usedCnt + Length >= MAX_BUF_LEN)
        {
            sliceCnt = MAX_BUF_LEN - cache->usedCnt;
        }

        // 累加内容
        memcpy((void*) &cache->data[cache->usedCnt], (void*) Data, sliceCnt);
        cache->usedCnt += sliceCnt;

        Log->PrintLog(DEBUG_LEVEL, "===Update:Port:%s Contentlen:%d UsedCnt:%d", key.c_str(), cache->cLen,
                cache->usedCnt);  // &&&

        // 判断值已接收完成
        int restSize = cache->cLen - cache->usedCnt;
        if (restSize != 0)
        {
            Log->PrintLog(DEBUG_LEVEL, "444444:Port:%s", key.c_str());  // &&&
            return;
        }

        Log->PrintLog(DEBUG_LEVEL, "Assembly contentLen:%d", cache->cLen);

        // 值接收完成
        this->RecvFinished(cache);

        // 回收节点
        mutex.lock();
        m_Mempool.FreeNode(*cache);
        flowtable.erase(it);
        lrukey.remove(key);
        mutex.unlock();
    }

    Log->PrintLog(DEBUG_LEVEL, "flow:%d lru:%d", flowtable.size(), lrukey.size());
    return;
}


bool PacketSniffer::RecvStart(CReqCache* cache, char* Data, int Length)
{
    char* p = Data;
    int j = 0;  // 报文起始头
    unsigned int field = 0;

    // 是否存在\r\n\r\n--，分离内容体
    for (int i = 0; i < Length; i++)
    {
        field = ntohl(*(unsigned int*) (p + i));

        switch (field)
        {
            case 0x0D0A2D2D:    // 起始处: \r\n--
            {
                j = i;
                break;
            }
        }
    }

    // 头与内容没有粘在一起
    if (j == 0)
    {
        return false;
    }

    // --
    j += 2;

    int sliceCnt = Length - j;
    if (sliceCnt >= MAX_BUF_LEN)
    {
        sliceCnt = MAX_BUF_LEN;
    }

    // 累加内容
    memcpy((void*) &cache->data[cache->usedCnt], (void*) &p[j], sliceCnt);
    cache->usedCnt += sliceCnt;
    return true;
}


bool PacketSniffer::RecvModify(char* Data, int Length)
{
    if (Length < 4)
    {
        return false;
    }

    char* p = Data;

    // {"os  }
    if (ntohl(*(unsigned int*) (p)) == 0x7B226F73 && (p[Length - 1] == '}'))
    {
        Log->PrintLog(DEBUG_LEVEL, "Out of order: %d", Length);
        SendMsg(p, Length);

        return true;
    }

    return false;
}


bool PacketSniffer::RecvFinished(CReqCache* cache)
{
    int len = cache->usedCnt;
    char* p = cache->data;
    unsigned int field = 0;

    int start = 0;
    int end = 0;
    int last = 0; // out of order

    // 逐个字段遍历
    for (int i = 0; i < len; i++)
    {
        field = ntohl(*(unsigned int*) (p + i));

        switch (field)
        {
            case 0x0D0A0D0A:    // 起始处: \r\n\r\n
            {
                start = i;
                break;
            }
            case 0x0D0A2D2D:    // 中间处: \r\n--
            {
                end = i;
                break;
            }
            case 0x2D2D0D0A:    // 结束处: --\r\n
            {
                last = i;
                break;
            }
        }
    }

    // 发送最终body
    if (start + 4 < len && end > 0)
    {
        start += 4;
        int realLen = end - start;

        // Modify the order
        if (realLen < 0)
        {
            start = last;
            if (start + 4 >= len)
            {
                return false;
            }

            start += 4;
            realLen = len - start;
        }

        // 拷贝性能损耗
        char* q = p + start;
        //std::string msg(q, realLen);

        // 解析完成并发送kafka
        Log->PrintLog(DEBUG_LEVEL, "Json:[%s]", &q);
        SendMsg(q, realLen);

        // 移除该包
        cache->cLen = 0;
        cache->usedCnt = 0;
        cache->data[0] = '\0';
        return true;
    }

    return false;
}


// Libpcap回调函数
static void GetPacket(u_char* arg, const struct pcap_pkthdr*, const u_char* packet)
{
    PacketSniffer* pSniffer = (PacketSniffer*) arg;
    pSniffer->HandleFrame((char*) packet);
}


bool PacketSniffer::InitKafka()
{
    if (CSysConfig::Inst()->dest_topic.empty())
    {
        Log->PrintLog(WARNING_LEVEL, "Disable kafka mode");
        return false;
    }

    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    RdKafka::Conf* tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
    std::string errstr;

    // conf
    conf->set("metadata.broker.list", CSysConfig::Inst()->kafka_host, errstr);
    conf->set("default_topic_conf", tconf, errstr);

    // 创建生产者
    m_producer = RdKafka::Producer::create(conf, errstr);
    if (!m_producer)
    {
        fprintf(stderr, "Failed to create producer: %s\n", errstr.c_str());
        return false;
    }

    fprintf(stderr, "Created producer: %s\n", m_producer->name().c_str());
    return true;
}


bool PacketSniffer::SendMsg(char* msg, int size)
{
    if (m_producer == nullptr)
    {
        return false;
    }

    RdKafka::ErrorCode resp = m_producer->produce(CSysConfig::Inst()->dest_topic,
            RdKafka::Topic::PARTITION_UA,
            RdKafka::Producer::RK_MSG_COPY,
            msg, size,
            NULL, 0,
            0, NULL);
    if (resp != RdKafka::ERR_NO_ERROR)
    {
        fprintf(stderr, "Produce failed: %s\n", RdKafka::err2str(resp).c_str());
        return false;
    }

    return true;
}


void PacketSniffer::PcapSniffer(const char* eth)
{
    char errBuf[1024] = {};
    pcap_t* device = pcap_open_live(eth, 65535, 1, 0, errBuf);
    if (!device)
    {
        Log->PrintLog(ERROR_LEVEL, "Open pcap error: %s", errBuf);
        exit(1);
    }

    char filterCmd[128] = {};
    sprintf(filterCmd, "dst port %d", CSysConfig::Inst()->port);

    // bpf过滤
    struct bpf_program filter;
    pcap_compile(device, &filter, filterCmd, 1, 0);
    pcap_setfilter(device, &filter);

    // 循环取包
    pcap_loop(device, -1, GetPacket, (u_char*) this);
}


bool PacketSniffer::LRUClear()
{
    // 使用量如果达到80%才清理
    if (m_Mempool.UsedNodesCount() < m_Mempool.NodesCount() * 4 / 5)
    {
        return false;
    }

    // 超过80%，需清理50%
    int clearCnt = m_Mempool.UsedNodesCount() / 2;
    int i = 0;
    Log->PrintLog(INFO_LEVEL, "LRU clear all:%d clear:%d", lrukey.size(), clearCnt);

    mutex.lock();
    for (auto it = lrukey.begin(); it != lrukey.end();)
    {
        // 已删除指定数目
        if (i++ > clearCnt)
        {
            break;
        }

        m_Mempool.FreeNode(*flowtable[*it]);
        flowtable.erase(*it);
        lrukey.erase(it++);
    }
    mutex.unlock();

    return true;
}
