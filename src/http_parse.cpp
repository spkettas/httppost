#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include "flowdef.h"


/**
 * 解析HTTP POST协议
 * @param data
 * @param len
 * @param info
 * @return
 */
bool ParseHttp(char* data, int len, URLInfo* info)
{
    if (data == NULL || len == 0 || info == NULL)
    {
        return false;
    }

    char* p = data;                 // 原始数据
    unsigned char flag = 0x0;       // 标识，记录解析字段数
    unsigned int field = 0;         // 属性字段
    int j = 0;                      // 填充索引
    int vLen = 0;                   // path有效长度

    for (int i = 0; i < len; i++)
    {
        field = ntohl(*(unsigned int*) (p + i));   // 取字段的int值，加速匹配

        switch (field)
        {
            case 0x47455420:    // GET /front.js HTTP 1.1\r\n
            {
                if (i + 5 >= len)
                {
                    continue;
                }

                i += 4;     // offset

                // proxy server
                char* p1 = p + i;

                // http: https:
                if ((p1[0] == 'h' && p1[1] == 't' && p1[2] == 't' && p1[3] == 'p')
                        && (p1[4] == ':' || p1[4] == 's'))
                {
                    return false;
                }

                if (p[i] != '/')
                {
                    return false;
                }

                i++;    // offset /

                j = 0;
                while (p[i] != '\r' && p[i] != '\0' && p[i] != ' ' && j < MAX_PATHLEN - 1)
                {
                    info->path[j++] = p[i];
                    i++;
                }

                if (j > 0)
                {
                    info->plen = j;
                    info->path[j] = '\0';
                }

                vLen = info->plen;

                // xxx?ver=2.0
                char* qLoc = NULL;

                if ((qLoc = strstr(info->path, "?")))
                {
                    info->qLoc = qLoc + 1;
                    //*qLoc = '\0';
                    i++;

                    vLen = qLoc - info->path;
                }

                // isHtml
                if ((vLen > 4) && (info->path[vLen - 1] == 'm')
                        && (info->path[vLen - 2] == 't')
                        && (info->path[vLen - 3] == 'h')
                        && (info->path[vLen - 4] == '.'))
                {
                    info->isHtml = true;
                }

                flag |= 0x01;
                break;
            }
            case 0x504F5354:        // POST /data.cgi HTTP 1.1\r\n
            {
                if (i + 6 >= len)
                {
                    continue;
                }

                i += 5;     // offset

                // proxy server
                char* p1 = p + i;

                // http: https:
                if ((p1[0] == 'h' && p1[1] == 't' && p1[2] == 't' && p1[3] == 'p')
                        && (p1[4] == ':' || p1[4] == 's'))
                {
                    return false;
                }

                if (p[i] != '/')
                {
                    return false;
                }

                i++;    // offset /

                j = 0;
                while (p[i] != '\r' && p[i] != '\0' && p[i] != ' ' && j < MAX_PATHLEN - 1)
                {
                    info->path[j++] = p[i];
                    i++;
                }

                if (j > 0)
                {
                    info->plen = j;
                    info->path[j] = '\0';
                }

                break;
            }
            case 0x436F6E74:        // Content-Length | Content-Type
            {
                if (i + 13 >= len)
                {
                    continue;
                }

                i += 12;
                bool bType = false;

                if (p[i] == ':')
                {
                    i += 1;
                    bType = true;
                }
                else if (p[i] == 't' && p[i + 1] == 'h' && p[i + 2] == ':')
                {
                    i += 3;
                    bType = false;
                }
                else
                {
                    break;
                }

                // 跳过空格
                while (p[i] == 0x20)
                {
                    i++;
                }

                j = 0;
                char szBuff[MAX_PATHLEN] = {};

                while (p[i] != '\r' && p[i] != '\0' && p[i] != ' ' && j < MAX_PATHLEN - 1)
                {
                    szBuff[j++] = p[i];
                    i++;
                }

                // 结果赋值
                if (bType)  // Content-Type
                {
                    int prop = ntohl(*(unsigned int*) (szBuff));

                    // 防止多个type覆写
                    if (flag == (flag | 0x03))
                    {
                        break;
                    }

                    if (prop == 0x6D756C74)         // multipart/form-data
                    {
                        info->contentType = TYPE_MULTIPART;
                        flag |= 0x03;
                    }
                    else if (prop == 0x6170706C)    // application/txt
                    {
                        info->contentType = TYPE_APPLICATION;
                        flag |= 0x03;
                    }
                    else                            // other
                    {
                        info->contentType = TYPE_UNKNOWN;
                        flag |= 0x03;
                    }
                }
                else    // Content-Length
                {
                    info->contentLen = atoi(szBuff);
                }

                break;
            }
            case 0x486F7374:        // Host
            {
                if (i + 9 >= len)
                {
                    continue;
                }

                i += 5;     // offset host

                // offset space
                while (p[i] == 0x20)
                    i++;

                j = 0;
                while (p[i] != '\r' && p[i] != '\0' && j < MAX_HOSTLEN - 1)
                {
                    info->host[j++] = p[i];
                    i++;
                }

                if (j > 0)
                {
                    info->hlen = j;
                    info->host[j] = '\0';
                }

                flag |= 0x02;
                break;
            }
            case 0x52656665:        // refer
            {
                if (i + 20 >= len)
                {
                    continue;
                }

                i += 8;     // offset refer

                // offset space
                while (p[i] == 0x20)
                    i++;

                if (p[i + 4] == ':')
                {
                    i += 7;  // offset http://
                }
                else if (p[i + 5] == ':')
                {
                    i += 8;  // offset https://
                }
                else
                {
                    return false;
                }

                j = 0;
                while (p[i] != '/' && p[i] != '\r' && p[i] != '\0' && j < MAX_HOSTLEN - 1)
                {
                    info->refer[j++] = p[i];        // 选取主域名，过滤路径
                    i++;
                }

                if (j > 0)
                {
                    info->rlen = j;
                    info->refer[j] = '\0';
                }

                flag |= 0x04;
                break;
            }
            case 0x55736572:        // User-Agent
            {
                // 已获取
                if (flag == (flag | 0x08))
                {
                    break;
                }

                if (i + 12 >= len)
                {
                    break;
                }

                i += 11;    // offset ua
                while (p[i] == 0x20)
                    i++;    // offset space

                j = 0;
                while (p[i] != '\r' && p[i] != '\0' && j < MAX_UALEN - 1)
                {
                    info->ua[j++] = p[i];
                    i++;
                }

                break;
            }
            default:
            {
                break;
            }
        }//swith()
    }//for()

    return true;
}

