#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <signal.h>
#include "sysapp.h"
#include "packet_sniffer.h"


SysApp* gApp = nullptr;


/**
 * @brief 信号处理函数
 *
 */
void SignalHandle(int sig)
{
    switch (sig)
    {
        case SIGUSR1:
        {
            gApp->Stop();
            break;
        }
        default:
        {
            break;
        }
    }
}


/**
 * @brief 系统入口
 *
 */
int main(int argc, char** argv)
{
    signal(SIGUSR1, SignalHandle);
    signal(SIGPIPE, SIG_IGN);

    // 启动框架
    gApp = new SysApp();
	gApp->Init();
    gApp->Start();

    // 事件循环
    while (gApp->RunFlag())
    {
        // LRU策略
        gApp->CheckLRU();

        sleep(1);
        //usleep(20 * 1000);
    }

    return 0;
}
