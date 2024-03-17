#ifndef __APOLLO_POLLER_H__
#define __APOLLO_POLLER_H__

#include <vector>
#include <unordered_map>
#include "timestamp.h"
namespace apollo
{

class Channel;
class EventLoop;

//抽象的IO复用对象
class Poller
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;
    virtual ~Poller()                = default;

     /**
     * @brief 统一的IO复用接口，等待活跃事件的到来
     * 
     * @param timeoutMs 超时时间 
     * @param activeChannels 活跃的Channel列表 
     * @return Timestamp 时间戳
     */
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    //更新Channel对象上的事件
    virtual void updateChannel(Channel* channel) = 0;

    //移除指定的Channel对象
    virtual void removeChannel(Channel* channel) = 0;

    //指定的Channel对象是否在当前Poller中
    bool hasChannel(Channel* channel) const;

    //返回默认的Poller对象
    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    // key-套接字描述符 value-fd所属的通道
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;


private:
    EventLoop* ownerLoop_; // 所属的事件循环
};



}











#endif