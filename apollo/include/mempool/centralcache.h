#ifndef __APOLLO_CENTRAL_CACHE_H__
#define __APOLLO_CENTRAL_CACHE_H__

#include "utilis.h"

namespace apollo
{
//中心缓存对象
class CentralCache
{
public:
    static CentralCache* getInstance() 
    {
        static CentralCache cache;
        return &cache;
    }

     /**
     * @brief 从CentralCache中获取一定数量的对象
     *
     * @param _start 传出参数，连续对象的起始地址
     * @param _end 传出参数，连续对象的末尾地址
     * @param _cnt 连续对象的个数
     * @param _size 单个对象的大小
     * @return 返回实际获取到的对象个数
     */
    size_t fetchRangeObj(void*& start, void*& end, size_t cnt, size_t size);

     /**
     * @brief 将一定数量的自由链表对象归还给对应的Span
     *
     * @param _start 连续对象的起始地址
     * @param _size 单个对象的大小
     */
    void releaseList(void* start, size_t size);

private:
    /**
     * @brief 获取一个非空的Span对象
     *
     * @param _list 从_list中获取非空对象
     * @param _size 对象的大小
     * @param _bucket_lock _list对象的桶锁
     * @return 返回非空的Span对象
     */
    Span* getOneSpan(SpanList& list, size_t size, std::unique_lock<std::mutex>& bucket_lock);

private:
    CentralCache()                               = default;
    CentralCache(const CentralCache&)            = delete;
    CentralCache& operator=(const CentralCache&) = delete;
    
private:
    SpanList spanlists_[kBucketSize];
};

}

#endif