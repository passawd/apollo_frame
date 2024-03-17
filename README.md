高性能的后端分布式服务器网络框架

- 使用 C++11 重写 muduo 网络库；
- 仿写 tcmalloc 的高并发内存池；
- 仿写 sl4j 的日志模块；
- 使用 protobuf 来进行 RPC 序列化/反序列化；
- 使用 Zookeeper 来作为分布式的注册和发现服务；
## 项目部署
### 开发环境

- OS：Ubuntu 22.04 LTS
- 构建工具：CMake 3.16.3
- 编译器：GCC 9.4.0
- 调试器：GDB 9.2
- 第三方模块：
    - [json.cpp](https://github.com/nlohmann/json)：3.11.2
    - protobuf：3.17.0
    - ZooKeeper：3.4.10
### 如何使用

首先需要通信双方约定消息格式，示例文件如下，其中 `option cc_generic_services = true` 必须开启：

```proto
syntax = "proto3";

package fixbug;

option cc_generic_services = true;

message ResultCode {
    int32 errcode = 1;
    bytes errmsg = 2;
}

message LoginRequest {
    bytes name = 1;
    bytes pwd = 2;
}

message LoginResponse {
    ResultCode result = 1;
    bool success = 2;
}

service UserServiceRpc {
    rpc Login(LoginRequest) returns(LoginResponse);
}
```

然后通过如下命令在当前路径下生成相应的头文件和源文件，该头文件中会包含两个类：`UserServiceRpc` 和 `UserServiceRpc_Stub`：

```sh
proto xxx.proto --cpp_out=./
```

对于服务提供方而言，需要继承 `UserServiceRpc` 类，并重写相应的方法，如下所示：

```c++
class UserService : public UserServiceRpc {
  bool Login(const std::string& name, const std::string& pwd) {
    // 实现业务方法...

    return true;
  }

  // 重新基类方法
  void Login(::google::protobuf::RpcController* controller,
    const ::fixbug::LoginRequest*             request,
    ::fixbug::LoginResponse*                  response,
    ::google::protobuf::Closure*              done) override {
    // 通过request参数获取请求方的函数参数
    std::string name = request->name();
    std::string pwd  = request->pwd();

    // 执行本地业务
    bool res = Login(name, pwd);

    // 将响应结果写入response参数
    fixbug::ResultCode* code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");
    response->set_success(res);

    // 执行回调操作
    done->Run();
  }
};

int main() {
  // 把UserService对象发布到RPC节点上
  RpcProvider provider;
  provider.notifyService(new UserService());

  // 启动一个rpc服务发布节点
  provider.run();

  return 0;
}
```

对于服务请求方而言，实例化一个 `UserServiceRpc_Stub` 对象，然后将函数参数放入到 request 对象中，然后通过 `UserService_Stub` 对象执行远程调用即可，最终通过 RpcControllerImpl 得知远程调用是否成功，如果成功则通过 response 对象取出服务请求的结果即可，示例代码如下：

```c++
int main() {
  fixbug::UserServiceRpc_Stub stub(new apollo::RpcChannelImpl);
  fixbug::LoginRequest        request;
  request.set_name("zhang san");
  request.set_pwd("123456");
  fixbug::LoginResponse response;
  // 发起RPC同步调用
  apollo::RpcControllerImpl controller;
  stub.Login(&controller, &request, &response, nullptr);

  if (controller.Failed()) {
      std::cout << controller.ErrorText() << std::endl;
  } else {
      // RPC调用完成 读取结果
      if (response.result().errcode() == 0) {
          std::cout << "rpc login response: " << response.success() << std::endl;
      } else {
          std::cout << "failed to rpc login: " << response.result().errmsg() << std::endl;
      }
  }
  return 0;
}
```
## 内存池模块
该项目所使用的内存池原型是 Google 的开源项目 tcmalloc，即Thread-Caching Malloc。线程缓存的 malloc ，实现了高效的多线程内存管理，用于替代malloc 和 free。

该内存池主要由三个部分组成
- `Thread Cache`: 线程缓存是每个线程独有的，用于小于等于 256KB 的内存分配
- `Central Cache`: 中心缓存是所有线程共享的，当 ThreadCache 需要内存时会按需从 CentralCache 中获取内存，而CentralCache 也会在合适的时机对 ThreadCache 中的内存进行回收。
- `Page Cache`: 页缓存中存储的内存是以页为单位进行存储及分配的，当 CentralCache 需要内存时，PageCache 会分配出一定数量的页给 CentralCache，而当 CentralCache 中的内存满足一定条件时，PageCache 也会在合适的时机对其进行回收，并将回收的内存尽可能的进行合并，组成更大的连续内存块，缓解内存碎片的问题。

上述三个部分的主要作用如下：

- Thread Cache: 主要解决锁竞争的问题；
- Central Cache: 主要负责居中调度的问题；
- Page Cache: 主要负责提供以页为单位的大块内存；

### 基数树
由于在 PageCache 中最初建立页号与 Span 之间的映射关系时，采用的是 unordered_map 数据结构，但是通过性能测试发现，内存池的性能并未优于原生的 malloc/free 接口。

这主要是因为 unordered_map 不是线程安全的，在多线程环境下需要加锁，而大量的加锁则会导致资源的消耗和性能的下降，因此在映射页号与 Span 之间的关系时，采用基数树（Radix Tree）数据结构来进行优化。

### 性能测试

单线程下内存池性能测试结果如下表所示，其中 `alloc/dealloc` 表示使用内存池来进行内存的申请和分配，而 `malloc/free` 表示使用系统原生的 API 来进行内存的申请和分配，表格中的单位为秒：

| 申请次数 | malloc   | free      | alloc    | dealloc  |
| -------- | -------- | -------- | -------- | -------- |
| 10000    | 0.06618  | 0.012673 | 0.018124 | 0.003872 |
| 30000    | 0.199733 | 0.043747 | 0.03611  | 0.011694 |
| 20000    | 0.120307 | 0.028152 | 0.024508 | 0.006907 |


多线程下的性能测试结果如下表所示，每个线程都需申请/释放内存 10000 次：

| 线程数目 | malloc   | free     | alloc    | dealloc  |
| -------- | -------- | -------- | -------- | -------- |
| 2        | 0.28304   | 0.07146 | 0.055868 | 0.017971 |
| 3        | 0.613199 | 0.194174 | 0.156596 | 0.03168  |
| 4        | 0.492761 | 0.060352 | 0.492761 | 0.060352 |
| 5        | 2.65943  | 0.416495 | 0.680774 | 0.092576 |

综合对比上面的测试数据，可以得出结论，当申请的内存次数较小时，使用系统原生的 API 更为合适，而如果申请次数过多时，内存池的优势就逐渐体现出来了。

## 网络通信模块

网络通信模块采用的是 muduo 网络库，本项目通过使用 C++11 简化 muduo 网络库，去除了 Boost 库的依赖以及一些冗余的组件，并提取出 muduo 库中的核心思想，即 One Loop Per Thread。

### 1. Reactor

该网络库采用的是 Reactor 事件处理模式。

- 既主线程（即 I/O 处理单元）只负责监听文件描述符上是否有事件发生，有的话就立即将该事件通知工作线程（即逻辑单元）。此外，主线程不做任何其他实质性的工作。读写数据、接受新的连接，以及处理客户请求均在工作线程中完成。

并发模式参考 muduo 库，采用半同步/半异步模式

- 既主线程对监听 socket 调用 epoll_wait() 函数，同时派发连接socket到工作线程，工作线程对连接 socket 调用 epoll_wait() 函数。

Multi Reactors - Multi Threads模型

-在这种模型中，主要分为两个部分：mainReactor、subReactors。 mainReactor 主要负责接收客户端的连接，然后将建立的客户端连接通过负载均衡的方式分发给 subReactors，subReactors 则负责具体的每个连接的读写，而对于非 IO 的操作，依然交给工作线程池去做，对逻辑进行解耦。
-该模型的核心在于，每一个 Reactor 都是一个 EventLoop 对象，而每一个 EventLoop 则和一个线程唯一绑定，MainLoop 只负责新连接的建立，连接建立成功后则将其打包为 TcpConnection 对象分发给 SubLoop。
-在 muduo 库中，使用了 eventfd() 代替 pipe 作为线程通信的手段

### 2. I/O multiplexing

在 Linux 系统下，常见的 I/O 复用机制有三种：select、poll 和 epoll。

其中，select 模型的缺点如下：

1. 单个进程能够监视的文件描述符的数量存在最大限制，通常是 1024，当然可以更改数量，但由于 select 采用轮询的方式扫描文件描述符，文件描述符数量越多，性能越差；
2. 内核和用户空间的内存拷贝问题，select 需要复制大量的句柄数据结构，会产生巨大的开销；
3. select 返回的是含有整个句柄的数组，应用程序需要遍历整个数组才能发现哪些句柄发生了事件；
4. select 的触发方式是水平触发，应用程序如果没有对一个已经就绪的文件描述符进行相应的 I/O 操作，那么之后每次 select 调用还是会将这些文件描述符通知进程；

相比于 select 模型，poll 则使用链表来保存文件描述符，因此没有了监视文件数量的限制，但其他三个缺点依然存在。

而 epoll 的实现机制与 select/poll 机制完全不同，它们的缺点在 epoll 模型上不复存在。其高效的原因有以下两点：

1. 它通过使用**红黑树**这种数据结构来存储 epoll 所监听的套接字。当添加或者删除一个套接字时（epoll_ctl），都是在红黑树上进行处理，由于红黑树本身插入和删除性能比较好，时间复杂度为 O(logN)，因此其效率要高于 select/poll。
2. 当把事件添加进来的时候时候会完成关键的一步，那就是该事件会与相应的设备（网卡）驱动程序建立回调关系，当相应的事件发生后，就会调用这个回调函数。这个回调函数其实就是把该事件添加到 `rdllist` 这个双向链表中。那么当我们调用 epoll_wait 时，epoll_wait 只需要检查 rdlist 双向链表中是否有存在注册的事件，效率非常可观。

epoll 对文件描述符的操作有两种模式：LT（Level Trigger，电平触发）和 ET（Edge Trigger，边沿触发）模式。其中，LT 模式是默认的工作模式，这种模式下 epoll 相当于一个效率较高的 poll。当往 epoll 内核事件表中注册一个文件描述符上的 EPOLLOUT 事件时，epoll 将以 ET 模式来操作该文件描述符。ET 模式是 epoll 的高效工作模式。

对于采用 LT 工作模式的文件描述符，当 epoll_wait 检测到其上有事件发生并将此事件通知应用程序后，应用程序可以不立即处理该事件。这样，当应用程序下一次调用 epoll_wait 时，epoll_wait 还会再次向应用程序通告此事件，直到该事件被处理。而对于采用 ET 工作模式的文件描述符，当 epoll_wait 检测到其上有事件发生并将此事件通知应用程序后，应用程序必须立即处理该事件，因为后续的 epoll_wait 调用将不再向应用程序通知这一事件。可见，ET 模式在很大程度上降低了同一个 epoll 事件被重复触发的此时，因此效率要比 LT 模式高。

在 muduo 网络库中，则采用了 LT 工作模式，其原因如下：

1. 不会丢失数据或者消息，应用没有读取完数据，内核是会不断上报的；
2. 每次读数据只需要一次系统调用；照顾了多个连接的公平性，不会因为某个连接上的数据量过大而影响其他连接处理消息；

在 muduo 网络库中，借助于 Linux 下“一切皆文件”的思想，通过 epoll 进行管理的主要有如下三个类型的事件：

- 网络 I/O 事件：通过套接字（socket）文件描述符进行管理；
- 线程通知事件：通过 eventfd 唤醒 SubLoop 处理相应的任务；
- 定时器事件：通过 timerfd 来处理定时器事件；

## 日志模块

具体包括一下几个类
- LogFormatter: 日志格式器，与log4cpp的PatternLayout对应，用于格式化一个日志事件。该类构建时可以指定pattern，表示如何进行格式化。提供format方法，用于将日志事件格式化成字符串。

- LogAppender: 日志输出器，用于将一个日志事件输出到对应的输出地。该类内部包含一个LogFormatter成员和一个log方法，日志事件先经过LogFormatter格式化后再输出到对应的输出地。从这个类可以派生出不同的Appender类型，比如StdoutLogAppender和FileLogAppender，分别表示输出到终端和文件。

- Logger: 日志器，负责进行日志输出。一个Logger包含多个LogAppender和一个日志级别，提供log方法，传入日志事件，判断该日志事件的级别高于日志器本身的级别之后调用LogAppender将日志进行输出，否则该日志被抛弃。

- LogEvent: 日志事件，用于记录日志现场，比如该日志的级别，文件名/行号，日志消息，线程/协程号，所属日志器名称等。

- LogManager: 日志器管理类，单例模式，用于统一管理所有的日志器，提供日志器的创建与获取方法。LogManager自带一个root Logger，用于为日志模块提供一个初始可用的日志器。


## RPC模块

该项目使用 Protobuf 来进行消息的序列化和反序列化，同时使用其来实现RPC框架，其底层的通信流程如下图所示：

![图片](https://github.com/passawd/apollo_frame/assets/162552756/20e1266c-1c52-4775-b98c-dc8aa5f4b666)

此外，为了解决TCP的粘包问题，我们设计了如下格式的数据头用来传递服务名称、方法名称以及参数大小，通过该数据头部我们可以确定所要读取的数据长度：

```proto
message RpcHeader {
  bytes service_name = 1;
  bytes method_name = 2;
  uint32 args_size = 3;
}
```

同时，为了确定 RpcHeader 的长度，我们使用了固定的 4 个字节来存储消息头的长度。数据打包和解包的流程如下所示：

**打包流程**：

1. 序列化函数参数得到 argsStr，其长度为 argsSize；
2. 打包 service_name、method_name 和 argsSize 得到 rpcHeader；
3. 序列化 rpcHeader 得到 rpcHeaderStr，其长度为 headerSize；
4. 将 headerSize 存储到数据包的前 4 个字节，后面的依次是 rpcHeaderStr 和 argsStr；
5. 通过网络发送数据包；

**解包流程**：

1. 通过网络接收数据包；
2. 首先取出数据包的前 4 个字节，读取出 headerSize 的大小；
3. 从第 5 个字节开始，读取 headerSize 字节大小的数据，即为 rpcHeaderStr 的内容；
4. 反序列化 rpcHeaderStr，得到 service_name、method_name 和 argsSize；
5. 从 4+headerSize 字节开始，读取 argsSize 字节大小的数据，即为 argsStr 的内容；
6. 反序列化 argsStr 得到函数参数 args；

### ZooKeeper

- 在分布式应用中，为了能够知道自己所需的服务位于哪台主机上，我们需要一个服务注册与发现中心，这也就是该项目中的ZooKeeper。它是一种分布式协调服务，可以在分布式系统中共享配置，协调锁资源，提供命名服务。

- Zookeeper通过树形结构来存储数据，它由一系列被称为ZNode的数据节点组成，类似于常见的文件系统。不过和常见的文件系统不同，Zookeeper将数据全量存储在内存中，以此来实现高吞吐，减少访问延迟。

- 由于 zookeeper_init 是异步方法，所以该函数返回后，并不代表句柄创建成功，它会在回调函数线程中调用所传入的回调函数，在该回调函数中我们可以来判断句柄是否创建成功，由于API调用线程阻塞于信号量处，所以在回调函数中如果句柄创建成功，则调用 sem_post 方法增加信号量，以通知API调用线程句柄创建成功






