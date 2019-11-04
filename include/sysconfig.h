#pragma once


#include <string>
#include <vector>
#include <map>


/**
 * @brief 配置解析类
 *
 * @author kanesun
 * @date 2019/7/17
 *
 */
class CSysConfig
{
private:
    CSysConfig();

    ~CSysConfig();

public:
    // 单例
    static CSysConfig* Inst();

    // 初始化
    bool Init(const char* file);

    // 读取配置
    bool ReadFile(const char* file, std::map<std::string, std::string>& map);


public:
    int daemon;     // 是否后台
    int loglevel;   // 日志等级
    std::vector<std::string> devs;     // 多采集网卡
    int port;               // 监控端口
    int poolsize;           // 内存池大小
    std::string kafka_host; // kafka服务器
    std::string dest_topic; // 流向topic
};


