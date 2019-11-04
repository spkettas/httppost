#include "sysapp.h"
#include "sysconfig.h"
#include <unistd.h>
#include "packet_sniffer.h"
#include "SysLog.h"


using namespace DebugUtil;


///
SysApp::SysApp()
        : bRun(false)
          , Log(nullptr)
{

}


SysApp::~SysApp()
{
    vThread.clear();

    for (auto& dev : sniffers)
    {
        delete dev;
        dev = nullptr;
    }

    sniffers.clear();

    if (Log)
    {
        delete Log;
        Log = nullptr;
    }
}

bool SysApp::Init()
{
    CSysConfig* pCfg = CSysConfig::Inst();

    // 加载配置
    bool pFlag = pCfg->Init("../conf/sys.cfg");
    if (!pFlag)
    {
        fprintf(stderr, "Load config error\n");
        return false;
    }

    fprintf(stderr, "Load config ok\n");

    // 初始化日志
    Log = new SysLog();
    Log->SetLogLevel(pCfg->loglevel);
    Log->OpenLogFile("../log/log_");
    Log->PrintLog(INFO_LEVEL, "Init log ok");

    // 后台化
    if (pCfg->daemon)
    {
        daemon(1, 1);
    }

    // 启动多个监听器
    for (const auto& dev : pCfg->devs)
    {
        PacketSniffer* pSniffer = new PacketSniffer(dev, Log);
        if (pSniffer == NULL)
        {
            fprintf(stderr, "Create sniffer error\n");
            break;
        }

        sniffers.push_back(pSniffer);
    }

    return true;
}


bool SysApp::Start()
{
    this->bRun = true;

    // 每个网卡启动一个线程
    for (const auto& pdev : sniffers)
    {
        std::thread th(&PacketSniffer::Start, pdev);

        vThread.push_back(std::move(th));
    }

    return true;
}


void SysApp::Stop()
{
    this->bRun = false;
}


void SysApp::CheckLRU()
{
    for (const auto& pdev : sniffers)
    {
        pdev->LRUClear();
    }
}

