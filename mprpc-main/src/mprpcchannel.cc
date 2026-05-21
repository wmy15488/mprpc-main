#include "mprpcchannel.h"
#include "mprpcapplication.h"
#include "logger.h"
#include  "zkclient.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

#include "mprpc.pb.h"

// 包头=mprpc.size[4字节]+mprpc(具体函数信息+请求信息的大小）+args（请求信息）
//负责把本地函数调用转换成远程调用
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
  (void)done;

  std::string method_name = method->name();
  const google::protobuf::ServiceDescriptor *service = method->service();
  std::string service_name = service->name();

  std::string args_str;
  if (!request->SerializeToString(&args_str))
  {
    MPRPC_LOG_ERR("serialize request error, service=%s method=%s",
                  service_name.c_str(),
                  method_name.c_str());
    controller->SetFailed("serialize request error");
    return;
  }
  mprpc::RpcHeader rpcHeader;
  rpcHeader.set_service_name(service_name);
  rpcHeader.set_method_name(method_name);
  rpcHeader.set_args_size(args_str.size());

  std::string rpcHeader_str;
  if (!rpcHeader.SerializeToString(&rpcHeader_str))
  {
    MPRPC_LOG_ERR("serialize rpc header error, service=%s method=%s",
                  service_name.c_str(),
                  method_name.c_str());
    controller->SetFailed("serialize rpc header error");
    return;
  }
  uint32_t header_size = rpcHeader_str.size();
  std::string send_rpc_str;
  send_rpc_str.insert(0, std::string((char *)&header_size, 4));
  send_rpc_str += rpcHeader_str;
  send_rpc_str += args_str;
//去zookeeper服务器查找调用函数所在服务器的ip与port
  ZkClient zkCli;
zkCli.Start();

std::string method_path = "/" + service_name + "/" + method_name;
std::string host_data = zkCli.GetData(method_path.c_str());

if (host_data.empty())
{
    controller->SetFailed("rpc service is not exist!");
    return;
}

int idx = host_data.find(':');
if (idx == -1)
{
    controller->SetFailed("rpc service address is invalid!");
    return;
}

std::string ip = host_data.substr(0, idx);
std::string port = host_data.substr(idx + 1);


  uint16_t port_ = static_cast<uint16_t>(std::stoi(port));
  int clientfd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (clientfd == -1)
  {
    MPRPC_LOG_ERR("create socket error, service=%s method=%s",
                  service_name.c_str(),
                  method_name.c_str());
    controller->SetFailed("create socket error");
    return;
  }
  sockaddr_in server_addr;
  memset(&server_addr,0,sizeof(server_addr));
  server_addr.sin_port=htons(port_);
  server_addr.sin_addr.s_addr=inet_addr(ip.c_str());
  server_addr.sin_family=AF_INET;
  if(::connect(clientfd,(sockaddr*)&server_addr,sizeof(server_addr))==-1)
  {
    MPRPC_LOG_ERR("connect server error, service=%s method=%s ip=%s port=%s",
                  service_name.c_str(),
                  method_name.c_str(),
                  ip.c_str(),
                  port.c_str());
    ::close(clientfd);
    controller->SetFailed("connect server error");
    return;
  }
  if(::send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0)==-1)
  {
    MPRPC_LOG_ERR("send rpc request error, service=%s method=%s",
                  service_name.c_str(),
                  method_name.c_str());
    ::close(clientfd);
    controller->SetFailed("send rpc request error");
    return;
  }
  
  char buf[1024]={0};
  int n=::recv(clientfd,buf,1024,0);
  if(n<0)
  {
    MPRPC_LOG_ERR("recv rpc response error, service=%s method=%s",
                  service_name.c_str(),
                  method_name.c_str());
    ::close(clientfd);
    controller->SetFailed("recv rpc response error");
    return;
  }
  if(!response->ParseFromArray(buf,n))
  {
    MPRPC_LOG_ERR("parse rpc response error, service=%s method=%s",
                  service_name.c_str(),
                  method_name.c_str());
    ::close(clientfd);
    controller->SetFailed("parse rpc response error");
    return;
  }

  ::close(clientfd);
}
