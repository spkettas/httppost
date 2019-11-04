#ifndef _PACKET_SNIFFER_H_
#define _PACKET_SNIFFER_H_


#include "flowdef.h"
#include <map>
#include <unordered_map>
#include <list>
#include <mutex>
#include "FixedContainer.h"
#include "SysLog.h"


using namespace Algorithm;

namespace RdKafka
{
    class Producer;
}

/**
 * @brief 网络数据包分析类，支持libpcap及pf_ring
 *
 * @author kanesun
 * @date 2019/7/17
 */
class PacketSniffer
{
public:
    PacketSniffer(const std::string& dev, DebugUtil::SysLog* Log);

    ~PacketSniffer();

    // 启动采集
    bool Start();

    // 处理数据包
    void HandleFrame(char* pdata);

    // LRU清理策略
    bool LRUClear();


private:
    // 初始化kafka
    bool InitKafka();

    // 开始接收值
    bool RecvStart(CReqCache* cache, char *Data, int Length);
    // 首包为body的直接发送
    bool RecvModify(char* Data, int Length);
    // 值接收完成
    bool RecvFinished(CReqCache* cache);

    // 发送kafka消息
    bool SendMsg(char* msg, int size);

    // Pcap采集
    void PcapSniffer(const char* eth);


private:
    DebugUtil::SysLog* Log;             // 日志打印类
    char netdev[32];                    // 采集网卡
    RdKafka::Producer* m_producer;      // kafka生产者

    std::mutex mutex;                   // 互斥锁
    Algorithm::FixedContainer<CReqCache> m_Mempool;  // 值内存池
    std::unordered_map<std::string, CReqCache*> flowtable;        // 上下文流表
    std::list<std::string> lrukey;                      // lru老化流表
};

#endif  // _PACKET_SNIFFER_H_
