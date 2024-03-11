#ifndef ROCKET_NET_RPC_RPC_CONTROLLER_H
#define ROCKET_NET_RPC_RPC_CONTROLLER_H

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <stdio.h>
#include <memory>
#include "/home/zxl/rocket/rocket/net/tcp/net_addr.h"

namespace rocket{

class RpcController : public google::protobuf::RpcController {

 public:
  typedef std::shared_ptr<RpcController> s_ptr;
  // Client-side methods ---------------------------------------------

  RpcController() = default;

  ~RpcController() = default;

  void Reset() override;

  bool Failed() const override;


  // Server-side methods ---------------------------------------------

  std::string ErrorText() const override;

  void StartCancel() override;

  void SetFailed(const std::string& reason) override;

  bool IsCanceled() const override;

  void NotifyOnCancel(google::protobuf::Closure* callback) override;


  // common methods

  int ErrorCode() const;

  void SetErrorCode(const int error_code);

  const std::string& MsgSeq() const;

  void SetMsgReq(const std::string& msg_req);

  void SetError(const int err_code, const std::string& err_info);

  void SetPeerAddr(NetAddr::s_ptr addr);

  void SetLocalAddr(NetAddr::s_ptr addr);

  NetAddr::s_ptr PeerAddr();
  
  NetAddr::s_ptr LocalAddr();

  void SetTimeout(const int timeout);

  int Timeout() const;

  void SetMethodName(const std::string& name);

  std::string GetMethodName();

  void SetMethodFullName(const std::string& name);

  std::string GetMethodFullName();



 private:
  int m_error_code {0};           // error_code, identify one specific error
  std::string m_error_info;       // error_info, details description of error
  std::string m_msg_req;          // msg_req, identify once rpc request and response
  bool m_is_failed {false}; 
  bool m_is_cancled {false};
  NetAddr::s_ptr m_peer_addr;
  NetAddr::s_ptr m_local_addr;

  int m_timeout {5000};           // max call rpc timeout
  std::string m_method_name;      // method name
  std::string m_full_name;        // full name, like server.method_name


};
}

#endif