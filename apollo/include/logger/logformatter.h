#ifndef __APOLLO_LOG_FORMATTER_H__
#define __APOLLO_LOG_FORMATTER_H__

#include "loglevel.h"
#include <memory>
#include <string>
#include <vector>
namespace apollo
{
    class Logger;
    class LogEvent;

class LogFormatter
{
public:
    LogFormatter(const std::string& pattern);

    /**
     * @brief 返回格式化日志文本
     *
     * @param logger 日志器
     * @param level 日志等级
     * @param ev 日志事件
     */
    std::string   format(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> ev);
    std::ostream& format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> ev);

public:
    //纯虚类，日志内容项格式化
    class FormatItem
    {
        FormatItem()          = default;
        virtual ~FormatItem() = default;

         /**
         * @brief 纯虚函数，格式化日志到流
         *
         * @param os 输出参数，日志输出流
         * @param logger 日志器
         * @param level 日志等级
         * @param ev 日志事件
         */
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger,
            LogLevel::Level level, std::shared_ptr<LogEvent> ev)
            = 0;
    };

     /// 初始化，解析日志模板
    void init();

    /// 是否有错误
    bool isError() const { return error_; }

    /// 返回日志模板
    const std::string& getPattern() const { return pattern_; }

    private:
    /// 日志格式模板
    std::string pattern_;
    /// 是否有错误
    bool error_;
    /// 日志格式解析后的格式
    std::vector<std::shared_ptr<FormatItem>> items_;

};

}

#endif