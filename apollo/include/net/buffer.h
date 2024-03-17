#ifndef __APOLLO_BUFFER_H__
#define __APOLLO_BUFFER_H__

#include <string>
#include <vector>

namespace apollo
{
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize  = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend) { }
    Buffer(const Buffer&) = default;
    Buffer& operator=(const Buffer&) = default;
    ~Buffer()                        = default;

    //返回可读的字节数
    size_t readableBytes() const { return writerIndex_ - readerIndex_; }

    //返回可写的字节数
    size_t writeableBytes() const { return buffer_.size() - writerIndex_; }

    //返回预留的字节数
    size_t prependabelBytes() const { return readerIndex_; }

    ///返回缓冲区中可读数据的起始地址
    const char* peek() const { return begin() + readerIndex_; }

    //读取长度为len的数据后移动读指针
    void retrieve(size_t len);

    //可读数据读完，恢复读指针和写指针
    void retrieveAll();

    //将缓冲区数据以std::string形式返回
    std::string retrieveAllAsString();

    //将缓冲区中长度为len的数据以std::string形式返回
    std::string retrieveAsString(size_t len);

    //确保可写区域至少有len个字节长度
    void ensureWritableBytes(size_t len);

    //向缓冲区中追加数据
    void append(const char* data, size_t len);

    //向缓冲区中追加数据
    void append(const std::string& data);

    //从fd中读取数据到缓冲区
    ssize_t readFd(int fd, int& saveErrno);

    //将缓冲区中的可读数据写入到fd中
    ssize_t writeFd(int fd, int& saveErrno);

    //与目标缓冲区进行交换
    void swap(Buffer& rhs);
    
private:
    /**
     * @brief 返回缓冲区的起始地址
     * 
     * @return char* 
     */
    char* begin() { return &*buffer_.begin(); }

    /**
     * @brief 返回缓冲区的起始地址
     * 
     * @return const char* 
     */
    const char* begin() const { return &*buffer_.begin(); }

    /**
     * @brief 返回写起始指针位置
     * 
     * @return char* 
     */
    char* beginWrite() { return begin() + writerIndex_; }

    /**
     * @brief 返回写起始指针位置
     * 
     * @return const char* 
     */
    const char* beginWrite() const { return begin() + writerIndex_; }

    /**
     * @brief 扩容到至少有len字节
     * 
     * @param len 
     */
    void makeSpace(size_t len);

private:
    std::vector<char> buffer_;      // 动态缓冲区
    size_t            readerIndex_; // 读指针
    size_t            writerIndex_; // 写指针
};





}








#endif