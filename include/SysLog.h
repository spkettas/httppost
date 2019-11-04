/**
@file SysLog.h
@brief 系统日志类
@note <PRE>
*******************************************************************************
模块名         : 系统日志类
文件名         : SysLog.h
相关文件       :

文件实现功能   : 系统日志类定义了五个外部接口，一个是接口是传入需要打印日志的
                 文件名，
	               一个接口是根据不同的级别打印日志，第三个是切换日志文件
	               第四个是设置全局日志级别
	               第五个是获取全局日志级别
-------------------------------------------------------------------------------
备注           : -
-------------------------------------------------------------------------------
******************************************************************************
</PRE>
*/


#ifndef _SYSLOG_H_
#define _SYSLOG_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

namespace DebugUtil
{

    /// 日志级别
    enum LogLevel
    {
        ERROR_LEVEL = 1,    ///< 错误级别
        WARNING_LEVEL = 2,  ///< 警告级别
        INFO_LEVEL = 3,     ///< 信息级别
        DEBUG_LEVEL = 4     ///< 调试级别
    };

    //这个宏可以在调试日志中增加行号
    #define DEBUG_LEVEL_WITH_LINE(format, ...) DEBUG_LEVEL, "%s %d " format, __FILE__, __LINE__, ##__VA_ARGS__
    #define LEVEL_WITH_LINE(level, format, ...) level, "%s %d " format, __FILE__, __LINE__, ##__VA_ARGS__

    /// 系统日志类
    class SysLog
    {
    public:
        /// 构造函数
        SysLog();

        /// 析构函数
        ~SysLog();

        /// 初始化，创建日志文件
        /// @param filename 日志文件的路径
        /// @return 打开成功返回true。路径不存在或创建文件失败的时候返回false。
        bool OpenLogFile(const char* filename);

        /// 系统运行过程中，打印日志
        /// @param LogLevel 日志级别
        /// @param format 格式化字符串
        /// @param ... 动态参数
        /// @return 打印日志是否成功
        bool PrintLog(int LogLevel, const char* format, ...);

        /// 系统运行过程中，切换日志文件接口。到下一天的时候，文件名变成下一天的。
        bool SwitchLogFile();

        /// 设置日志级别
        /// @param Loglevel 日志级别 \see LogLevel
        bool SetLogLevel(int Loglevel);

        /// 设置日志级别，使用字符串ERROR WARNING INFO DEBUG
        bool SetLogLevel(const char* str);

        /// 获取日志级别
        int GetLogLevel();

    private:
        SysLog(const SysLog& rsh);

        SysLog& operator=(const SysLog& rsh);
        //-----------------------------------------------------------------------------
    private:
        FILE* m_Logfp;
        char m_FileName[100];
        int m_LogLevel;
        pthread_mutex_t m_LogLocker;
        int m_DayOfYear;
    };

};  //end namespace DebugUtil

#endif
