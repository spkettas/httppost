#ifndef _SYS_APP_H_
#define _SYS_APP_H_


#include <list>
#include <thread>
#include <vector>


class PacketSniffer;
namespace DebugUtil
{
    class SysLog;
}


/**
 * @brief 应用启动框架
 *
 * @author kanesun
 * @date 2019/7/17
 *
 */
class SysApp
{
public:
    SysApp();

    ~SysApp();

    // 加载配置
    bool Init();

    // 启动
    bool Start();

    // 关闭采集
    void Stop();

    // 是否运行
    inline bool RunFlag()
    {
        return bRun;
    }


    // LRU策略清除老化流表
    void CheckLRU();


private:
    DebugUtil::SysLog* Log;                         // 系统日志
    volatile bool bRun;                    // 运行标志
    std::list<PacketSniffer*> sniffers;    // 多个嗅探器
    std::vector<std::thread> vThread;     // 多个线程
};

#endif // _SYS_APP_H_

