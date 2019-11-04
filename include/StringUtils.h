#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <string.h>

using namespace std;

#ifndef _STRUTIL
#define STR     std::to_string
#endif

class StringUtils
{
public:
    StringUtils()
    {
    }

    virtual ~StringUtils()
    {
    }

    static uint32_t converStringToUint32(const string& dataStr)
    {
        stringstream ss;
        ss << dataStr;
        uint32_t data;
        ss >> data;
        return data;
    }

    static uint64_t converStringToUint64(const string& dataStr)
    {
        stringstream ss;
        ss << dataStr;
        uint64_t data;
        ss >> data;
        return data;
    }

    static int converStringToInt(const string& dataStr)
    {
        stringstream ss;
        ss << dataStr;
        int data;
        ss >> data;
        return data;
    }

    //字符串分割函数
    static int split(std::string str, std::string pattern,
            std::vector<std::string>& result)
    {
        str += pattern; //扩展字符串以方便操作
        for (size_t i = 0; i < str.size(); i++)
        {
            std::string::size_type pos = str.find(pattern, i);
            if (pos < str.size())
            {
                std::string s = str.substr(i, pos - i);
                result.push_back(s);
                i = pos + pattern.size() - 1;
            }
        }
        return 0;
    }

    //字符串分割函数
    static int split(std::string str, const char* pattern,
            std::vector<std::string>& result)
    {
        string patternStr(pattern);
        return split(str, patternStr, result);
    }

    static int split(const char* str, const char* pattern,
            std::vector<std::string>& result)
    {
        string strs(str);
        string patternStr(pattern);
        return split(strs, patternStr, result);
    }

    static void replaceAll(const string& oldStr, const string& newStr,
            string& src)
    {
        size_t npos = src.find(oldStr);
        while (npos != string::npos)
        {
            src.replace(npos, oldStr.length(), newStr);
            npos = src.find(oldStr);
        }
    }

    static void replaceAll(const char* old, const char* newChar, string& src)
    {
        string oldStr(old);
        string newStr(newChar);
        replaceAll(oldStr, newStr, src);
    }


    // Removes \r\n from the end of line
    inline static
    void FIXLINE(char* s)
    {
        size_t len = strlen(s) - 1;
        if (s[len] == '\n' || s[len] == '\r')
        {
            s[len] = '\0';
        } // "\n" for *nix files
        if (s[len - 1] == '\r')
        {
            s[len - 1] = '\0';
        } // "\r\n" for windows files
    }

    inline static
    void FIXBOM(char* s)
    {
        size_t len = strlen(s) - 1;
        if (len >= 3 && s[0] == '\xef' && s[1] == '\xbb' && s[2] == '\xbf')
        {
            for (size_t i = 0; i < len - 1; ++i)
            {
                s[i] = s[i + 3];
            }
        }
    }
};

#endif // _STRING_UTILS_H_

