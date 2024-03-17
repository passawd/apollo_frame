#ifndef __APOLLO_LOGGER_H__
#define __APOLLO_LOGGER_H__

#include "loglevel.h"
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace apollo
{
class LogEvent;
class LogAppender;
class LogFormatter;

//日志器
class Logger : public std::enable_shared_from_this<Logger>
{
public:

    Logger(const std::string& name = "root");

    void log(LogLevel::Level level, std::shared_ptr<LogEvent> ev);

    void debug(std::shared_ptr<LogEvent> ev);

    void info(std::shared_ptr<LogEvent> ev);

    void warn(std::shared_ptr<LogEvent> ev);

    void error(std::shared_ptr<LogEvent> ev);

    void fatal(std::shared_ptr<LogEvent> ev);

    //添加日志目标
    void addAppender(std::shared_ptr<LogAppender> appender);

    //删除日志目标
    void removeAppender(std::shared_ptr<LogAppender> appender);

    //清空日志目标
    void clearAppender();

    //返回日志等级
    LogLevel::Level getLevel() const { return level_; }

    //设置日志级别
    void setLevel(LogLevel::Level lv) { level_ = lv; }

    //返回日志名称
    const std::string& getName() const { return name_; }

    //设置日志格式器
    void setFormatter(std::shared_ptr<LogFormatter> fmt);

    //设置日志格式模板
    void setFormatter(const std::string& pattern);

    //获取日志格式器
    std::shared_ptr<LogFormatter> getFormatter();

private:
    /// 日志名称
    std::string name_;
    /// 日志级别
    LogLevel::Level level_;
    /// 日志目标集合
    std::list<std::shared_ptr<LogAppender>> appenders_;
    /// 日志格式器
    std::shared_ptr<LogFormatter> formatter_;
    /// 主日志器
    std::shared_ptr<Logger> root_;
    /// 互斥锁
    std::mutex mtx_;

    friend class LoggerManager;   
};

//日志管理器类
class LoggerManager
{
public:
    static LoggerManager* getInstance() 
    {
        static LoggerManager mgr;
        return &mgr;
    }

    //获取日志器
    std::shared_ptr<Logger> logger(const std::string& name);

    //获取主日志器
    std::shared_ptr<Logger> root() const { return root_; }

private:
    LoggerManager();

    /**
     * @brief 加载配置文件
     */
    void loadConfig();

private:
    /// 互斥锁
    std::mutex mtx_;
    /// 日志管理器容器
    std::map<std::string, std::shared_ptr<Logger>> loggers_;
    /// 主日志器
    std::shared_ptr<Logger> root_;    
    
};

}




#endif