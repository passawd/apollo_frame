#ifndef __APOLLO_THREAD_CACHE_H__
#define __APOLLO_THREAD_CACHE_H__


#include "utilis.h"

namespace apollo
{
class ThreadCache
{
public:
    ThreadCache()                              = default;
    ThreadCache(const ThreadCache&)            = delete;
    ThreadCache& operator=(const ThreadCache&) = delete;

     /**
     * @brief 申请内存对象
     */
    void* allocate(size_t size);

    /**
     * @brief 释放内存对象
     *
     * @param _ptr 要释放的内存对象
     * @param _size 对象的大小
     */
    void deallocate(void* ptr, size_t size);

private:
    //向CentralCache申请对象
    void* fetchFromCentralCache(size_t index, size_t size);

    //将FreeList对象归还给CentralCache
    void revertListToCentralCache(FreeList& list, size_t size);

private:
    FreeList freelists_[kBucketSize];

};




}















#endif