#ifndef __APOLLO_TIMESTAMP_H__
#define __APOLLO_TIMESTAMP_H__

#include <cstdint>
#include <string>

namespace apollo 
{

//时间戳类
class Timestamp 
{
public:

    Timestamp();

    explicit Timestamp(int64_t microSecondsSinceEpoch);

    //返回当前时间戳
    static Timestamp now();

    //返回一个无效的时间戳对象
    static Timestamp invalid() { return Timestamp(); }

    //对象是否有效
    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    //将时间戳转化为字符串形式
    std::string toString() const;

    //获取微秒数
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    //每秒的微秒数
    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) 
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) 
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline Timestamp addTime(Timestamp timestamp, double seconds) 
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}
} 

#endif