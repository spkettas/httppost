// -*- coding: utf-8-unix; -*-
// Copyright (c) 2019 Tencent, Inc.
//     All rights reserved.
//
// Author: kanesun@tencent.com
// Date: 2019/7/18 14:36
// Description:
//

#pragma once

#include <stdint.h>
#include <arpa/inet.h>
#include <string>


// URL长度
#define MAX_HOSTLEN         60
#define MAX_PATHLEN         256
#define MAX_UALEN           128
#define MAX_BUF_LEN (512*1024)    // 缓存区大小

// 内容类型
#define TYPE_UNKNOWN        0   // 未知
#define TYPE_MULTIPART      1   // multipart
#define TYPE_APPLICATION    2   // application


// 协议定义
#ifndef ETHERTYPE_VLAN
#define ETHERTYPE_VLAN          0x8100
#endif
#ifndef VALUE_GET
#define VALUE_GET                0x47455420
#endif


// 五元组
typedef struct _CFlowKey
{
    uint32_t srcip;
    uint16_t srcport;
    uint32_t dstip;
    //uint16_t dstport;     // 默认都是80,节省空间不需要存储

    _CFlowKey(uint32_t srcip, uint16_t srcport, uint32_t dstip)
    {
        this->srcip = srcip;
        this->srcport = srcport;
        this->dstip = dstip;
    }

    bool operator==(const _CFlowKey& key1)
    {
        return (srcip == key1.srcip)
                && (srcport == key1.srcport)
                && (dstip == key1.dstip);
    }

    // map要用到
    bool operator<(const _CFlowKey& other) const
    {
        return srcip < other.srcip
                || srcport < other.srcport
                || dstip < other.dstip;
    }

    std::string tostring()
    {
	char szbuff[32] = {};
	snprintf(szbuff, sizeof(szbuff) - 1,
		"%u|%X|%X",
		ntohs(srcport), srcip, dstip);

	return std::string(szbuff);
    }
} CFlowKey;


// 生成字符串key
#define CREATE_FLOWKEY(ip, tcp) \
	char szBuff[32] = {}; \
	snprintf(szBuff, sizeof(szBuff) - 1, \
		"%u|%X|%X", \
		ntohs(tcp->source), ip->saddr, ip->daddr); \
	std::string key(szBuff); \


// post请求包
typedef struct _CReqCache
{
    uint16_t cLen;            // 消息体长度
    uint16_t usedCnt;         // json体长度
    char data[MAX_BUF_LEN];   // json体
} CReqCache;


// hash算法
static unsigned int BKDRHashUsername(char* str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
    while (*str)
    {
        hash = hash * seed + (*str++);
    }

    return hash;
}


/**
 * 域名结构体
 */
typedef struct _URLInfo
{
    char host[MAX_HOSTLEN];         // 主域名
    int hlen;                       // host length
    char path[MAX_PATHLEN];         // 请求文件名称
    int plen;                       // path length
    char refer[MAX_HOSTLEN];        // refer
    int rlen;                       // refer length
    char ua[MAX_UALEN];             // UserAgent
    char* qLoc;                     // 刷新部分
    bool isHtml;                    // 是否为html

    uint8_t contentType;            // 内容体类型，见Type定义
    int contentLen;                 // 内容体长度
} URLInfo;


