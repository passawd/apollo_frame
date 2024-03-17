#ifndef __APOLLO_PAGE_CACHE_H__
#define __APOLLO_PAGE_CACHE_H__

#include "utilis.h"
#include "radixtree.h"
#include <mutex>

namespace apollo
{
    //页缓存对象
class PageCache
{
public:
    static PageCache* getInstance() 
    {
        static PageCache cache;
        return &cache;
    }

    //获取一个_npage页的Span对象
    Span* newSpan(size_t npage);

    //获取对象到Span的映射
    Span* mapToSpan(void* obj);

    //释放空闲的Span到PageCache 并合并相邻的Span
    void revertSpanToPageCache(Span* span);

private:
    PageCache()                            = default;
    PageCache(const PageCache&)            = delete;
    PageCache& operator=(const PageCache&) = delete;

public:
    std::mutex mtx_;

private:
    SpanList spanlists_[kPageBucketSize];
    // std::unordered_map<page_t, Span*> hash_;
    PageMap          hash_;
    ObjectPool<Span> span_pool_;
};
}














#endif