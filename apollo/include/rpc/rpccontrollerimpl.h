#ifndef __APOLLO_RPCCONTROLLERIMPL_H__
#define __APOLLO_RPCCONTROLLERIMPL_H__

#include <google/protobuf/service.h>

namespace apollo
{
class RpcControllerImpl : public google::protobuf::RpcController
{
public:
    RpcControllerImpl();
    ~RpcControllerImpl();

    void Reset() override;

    bool Failed() const override;

    std::string ErrorText() const override;

    void SetFailed(const std::string& reason) override;

    void StartCancel() override;

    bool IsCanceled() const override;

    //取消RPC调用时进行通知 
    void NotifyOnCancel(google::protobuf::Closure* callbak) override;

private:
    bool        failed_;  // RPC方法执行过程中的状态
    std::string errText_; // RPC方法执行过程中的错误信息
};
}
#endif