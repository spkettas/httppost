#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <signal.h>
#include "sysapp.h"
#include "packet_sniffer.h"


SysApp* gApp = nullptr;


/**
 * @brief �źŴ�����
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
 * @brief ϵͳ���
 *
 */
int main(int argc, char** argv)
{
    signal(SIGUSR1, SignalHandle);
    signal(SIGPIPE, SIG_IGN);

    // �������
    gApp = new SysApp();
	gApp->Init();
    gApp->Start();

    // �¼�ѭ��
    while (gApp->RunFlag())
    {
        // LRU����
        gApp->CheckLRU();

        sleep(1);
        //usleep(20 * 1000);
    }

    return 0;
}
