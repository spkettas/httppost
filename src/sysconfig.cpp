#include "sysconfig.h"
#include <string.h>
#include <assert.h>
#include <map>
#include "StringUtils.h"


///
CSysConfig* CSysConfig::Inst()
{
    static CSysConfig cfg;
    return &cfg;
}


CSysConfig::CSysConfig()
        : daemon(0)
          , kafka_host("")
          , dest_topic("")
{

}


CSysConfig::~CSysConfig()
{

}


bool CSysConfig::Init(const char* file)
{
    std::map<std::string, std::string> map;

    // 加载配置
    bool pFlag = ReadFile(file, map);
    if (!pFlag)
    {
        fprintf(stderr, "Read [%s] error", file);
        return false;
    }

    // 解析字段值
    for (const auto& pair : map)
    {
        if (pair.first == "daemon")
        {
            daemon = atoi(pair.second.c_str());
        }
        else if (pair.first == "loglevel")
        {
            loglevel = atoi(pair.second.c_str());
        }
        else if (pair.first == "dev")
        {
            StringUtils::split(pair.second, "|", devs);
        }
        else if (pair.first == "port")
        {
            port = atoi(pair.second.c_str());
        }
        else if (pair.first == "poolsize")
        {
            poolsize = atoi(pair.second.c_str());
        }
        else if (pair.first == "kafka_host")
        {
            kafka_host = pair.second;
        }
        else if (pair.first == "dest_topic")
        {
            dest_topic = pair.second;
        }
    }

    return true;
}


bool CSysConfig::ReadFile(const char* file, std::map<std::string, std::string>& map)
{
    map.clear();

    FILE* fp = fopen(file, "r");
    assert(fp != NULL);

    char szbuff[512] = {};
    std::map<std::string, std::string> lines;

    char szKey[128] = {};
    char szVal[128] = {};

    // 读取文件
    while (fgets(szbuff, sizeof(szbuff), fp))
    {
        if (!strncmp(szbuff, "#", 1)
                || !strncmp(szbuff, "\n", 1))
        {
            continue;
        }

        szVal[0] = '\0';
        sscanf(szbuff, "%[^=]=%s\n", szKey, szVal);
        map.insert({ szKey, szVal });
    }

    fclose(fp);
    return true;
}
