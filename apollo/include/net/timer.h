#ifndef __APOLLO_TIMER_H__
#define __APOLLO_TIMER_H__

#include "callbacks.h"
#include "timestamp.h"
#include <atomic>
namespace apollo
{
class Timer
{
public:
    /**
     * @brief Construct a new Timer object
     * 
     * @param cb 定时器回调函数
     * @param when 定时器到期时间
     * @param interval 定时器触发的时间间隔
     */
    Timer(TimerCallback cb, Timestamp when, double interval);
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    ~Timer()                       = default;

    //运行定时器的回调函数
    void run() const { callback_(); }

    //返回定时器的到期时间点
    Timestamp expiration() const { return expiration_; }

    //是否重复触发
    bool repeat() const { return repeat_; }

    //返回定时器的序号
    int64_t sequence() const { return sequence_; }

    //重新启动定时器
    void restart(Timestamp now);

    //返回定时器数量
    static int64_t numCreated() { return numCreated_; }

private:
    const TimerCallback callback_;   // 回调函数
    Timestamp           expiration_; // 定时器的到期时间
    const double        interval_;   // 定时器触发的时间间隔
    const bool          repeat_;     // 定时器是否重复
    const int64_t       sequence_;   // 定时器序号

    static std::atomic_int64_t numCreated_; // 定时器创建数目
};
}








#endif