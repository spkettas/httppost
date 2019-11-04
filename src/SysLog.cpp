#include "SysLog.h"
#include <assert.h>

using namespace DebugUtil;

const char* LOG_LEVEL_STR[] =
        {
                "ERROR",
                "WARNING",
                "INFO",
                "DEBUG"
        };

SysLog::SysLog()
        :
        m_Logfp(NULL), m_LogLevel(INFO_LEVEL), m_DayOfYear(0)
{
    pthread_mutex_init(&m_LogLocker, NULL);
    m_FileName[0] = '\0';
}

SysLog::~SysLog()
{
    if (NULL != m_Logfp)
    {
        fclose(m_Logfp);
        m_Logfp = NULL;
    }
}

bool SysLog::OpenLogFile(const char* filename)
{
    assert(NULL != filename);
    if (NULL == filename)
    {
        return false;
    }
    if (NULL != m_Logfp)
    {
        return false;
    }
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char str[20];
    strftime(str, sizeof(str), "%Y-%m-%d.log", t);
    str[sizeof(str) - 1] = '\0';

    char temp[120];
    snprintf(temp, sizeof(temp) - 1, "%s%s", filename, str);
    temp[sizeof(temp) - 1] = '\0';

    pthread_mutex_lock(&m_LogLocker);
    m_Logfp = fopen(temp, "a");
    if (NULL == m_Logfp)
    {
        pthread_mutex_unlock(&m_LogLocker);
        return false;
    }
    strncpy(m_FileName, filename, sizeof(m_FileName) - 1);
    m_FileName[sizeof(m_FileName) - 1] = '\0';
    m_DayOfYear = t->tm_yday;
    pthread_mutex_unlock(&m_LogLocker);
    return true;
}


bool SysLog::PrintLog(int LogLevel, const char* format, ...)
{
    if (LogLevel < ERROR_LEVEL || LogLevel > DEBUG_LEVEL)
    {
        return false;
    }
    if (NULL == m_Logfp)
    {
        if (!SwitchLogFile())
        {
            return false;
        }
    }
    if (LogLevel > m_LogLevel)
    {
        return false;
    }
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    if (m_DayOfYear != t->tm_yday)
    {
        if (!SwitchLogFile())
        {
            return false;
        }
    }

    char str[20];
    strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", t);
    str[sizeof(str) - 1] = '\0';

    pthread_mutex_lock(&m_LogLocker);
    assert(NULL != m_Logfp);
    fprintf(m_Logfp, "%s %s ", str, LOG_LEVEL_STR[LogLevel - 1]);
    va_list args;
    va_start(args, format);
    vfprintf(m_Logfp, format, args);
    va_end(args);
    fprintf(m_Logfp, "\n");
    fflush(m_Logfp);
    pthread_mutex_unlock(&m_LogLocker);
    return true;
}

bool SysLog::SwitchLogFile()
{
    pthread_mutex_lock(&m_LogLocker);
    if (NULL != m_Logfp)
    {
        fclose(m_Logfp);
        m_Logfp = NULL;
    }
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char str[50] = { 0 };
    strftime(str, 49, "%Y-%m-%d.log", t);
    //str[ sizeof(str)-1 ] = '\0'; //曾经core到这行

    char temp[256] = { 0 };
    snprintf(temp, 255, "%s%s", m_FileName, str);
    //temp[ sizeof(temp)-1 ] = '\0'; //曾经core到这行

    m_Logfp = fopen(temp, "a");
    if (NULL == m_Logfp)
    {
        pthread_mutex_unlock(&m_LogLocker);
        return false;
    }
    m_DayOfYear = t->tm_yday;
    pthread_mutex_unlock(&m_LogLocker);
    return true;
}


bool SysLog::SetLogLevel(int Loglevel)
{
    if (Loglevel < ERROR_LEVEL || Loglevel > DEBUG_LEVEL)
    {
        return false;
    }
    m_LogLevel = Loglevel;
    return true;
}

int SysLog::GetLogLevel()
{
    return m_LogLevel;
}

bool SysLog::SetLogLevel(const char* str)
{
    const int COUNT = 4;
    for (int i = 0; i < COUNT; i++)
    {
        if (0 == strcmp(str, LOG_LEVEL_STR[i]))
        {
            m_LogLevel = i + 1;
            return true;
        }
    }
    return false;
}

/**
* @example TestSysLog.cpp
*/
