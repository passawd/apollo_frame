#ifndef __APOLLO_CONCURRENT_ALLOC_H__
#define __APOLLO_CONCURRENT_ALLOC_H__

#include "pagecache.h"
#include "threadcache.h"
#include <cassert>

void* operator new(size_t size);
void* operator new[](size_t size);
void  operator delete(void* ptr) noexcept;
void  operator delete[](void* ptr) noexcept;
namespace apollo
{

static TLS ThreadCache* s_tlsthreadcache = nullptr;


static void* concurrentAlloc(size_t size)
{
    if(size> kMaxBytes)
    {
        size_t alignsize = AlignHelper::roundUp(size);

        size_t npage     = alignsize >> kPageShift;

        // 向PageCache申请npage页的span

        Span* span = nullptr;
        {
            PageCache* cache = PageCache::getInstance();
            std::lock_guard<std::mutex> lock(cache->mtx_);
            span             = cache->newSpan(npage);
            span->used_      = true;
            span->blockSize_ = size;
        }

        assert(span);

        void *ptr = (void *)(span->pageId_ <<kPageShift);
        return ptr;
    }
    else
    {
        if(s_tlsthreadcache==nullptr)
        {
            static std::mutex mtx;
            static ObjectPool<ThreadCache>tcpool;
            {
                std::lock_guard<std::mutex> lock(mtx);
                s_tlsthreadcache = tcpool.alloc();
            }
        }

        return s_tlsthreadcache->allocate(size);
    }
}

static void concurrentFree(void* ptr)
{
    if(ptr!=nullptr)
    {
        Span*  span = PageCache::getInstance()->mapToSpan(ptr);
        size_t size=span->blockSize_;

        if (size > kMaxBytes)
        {
            PageCache *cache =PageCache::getInstance();

            std::lock_guard<std::mutex> lock(cache->mtx_);

            cache->revertSpanToPageCache(span);
        }
        else
        {
            assert(s_tlsthreadcache);
            s_tlsthreadcache->deallocate(ptr, size);
        }
    }
}


}




#endif