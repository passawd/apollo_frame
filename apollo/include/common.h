#ifndef __APOLLO_COMMON_H__
#define __APOLLO_COMMON_H__

#include <fstream>
#include <ios>
#include <string>
#include <thread>
namespace apollo
{

class ThreadHelper
{
public:
    //获取线程id
    static pid_t ThreadId();

    //获取线程名称
    static const std::string& ThreadName();

    //设置线程名称
    static void SetThreadName(std::thread* th, const std::string& name);
};

class FileHelper
{
public:
    //创建文件夹
    static bool Mkdir(const std::string& dirname);

    //获取路径信息
    static std::string Dirname(const std::string& filename);

    /**
     * @brief 以写方式打开文件
     *
     * @param os 文件流
     * @param filename 文件名称
     * @param mode 打开方式
     * @return true 打开成功
     * @return false 打开失败
     */
    static bool OpenForWrite(std::ofstream& os, const std::string& filename, std::ios_base::openmode mode);
};




}

#endif