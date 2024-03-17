#ifndef __APOLLO_ZKCLIENT_H__
#define __APOLLO_ZKCLIENT_H__

#include <string>
#include <zookeeper/zookeeper.h>

namespace apollo
{
//ZooKeeper客户端类
class ZkClient
{
    ZkClient();
    ZkClient(const ZkClient&) = delete;
    ZkClient& operator=(const ZkClient&) = delete;
    ~ZkClient();

    void start();

    /**
     * @brief 创建Znode节点
     * 
     * @param path 路径
     * @param data 节点数据
     * @param flags 节点标志，可以是如下参数
     * 0: 表示永久性节点，默认情况
     * ZOO_EPHEMERAL: 临时性节点
     * ZOO_SEQUENCE: 路径名后添加唯一的、单调递增的序号
     */
    void create(const std::string& path, const std::string& data, int flags = 0);

    std::string getData(const std::string& path);
private:
    zhandle_t* zkHandler_; // zookeeper句柄
};

}


#endif