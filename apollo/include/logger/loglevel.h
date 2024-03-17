#ifndef __APOLLO_LOG_LEVEL_H__
#define __APOLLO_LOG_LEVEL_H__

#include <string>
namespace apollo
{

class LogLevel
{
public:
    enum Level 
    {
        UNKNOW = 0, /// 未知
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    //将日志级别转成文本输出
    static const char* ToString(LogLevel::Level level);

    //将文本转换成日志级别
    static LogLevel::Level FromString(const std::string& str);
};

















}
#endif