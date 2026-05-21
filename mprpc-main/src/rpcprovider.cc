#include "rpcprovider.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <mymuduo/Buffer.h>
#include <mymuduo/InetAddress.h>
#include <mymuduo/TcpServer.h>
#include <mymuduo/Timestamp.h>
#include <mprpc.pb.h>
#include <cstdlib>
#include <cstring>
#include <functional>
#include "zkclient.h"
#include "mprpcapplication.h"
#include "logger.h"

namespace
{

  class RpcProviderClosure : public google::protobuf::Closure
  {
  public:
    RpcProviderClosure(RpcProvider *provider,
                       const TcpConnection::TcpConnectionPtr &conn,
                       google::protobuf::Message *request,
                       google::protobuf::Message *response)
        : provider_(provider),
          conn_(conn),
          request_(request),
          response_(response)
    {
    }

    void Run() override
    {
      provider_->SendRpcResponse(conn_, response_);
      delete request_;
      delete response_;
      delete this;
    }

  private:
    RpcProvider *provider_;
    TcpConnection::TcpConnectionPtr conn_;
    google::protobuf::Message *request_;
    google::protobuf::Message *response_;
  };

} // namespace

RpcProvider::RpcProvider()
{
  std::string ip = MprpcApplication::GetConfig().Load("rpcserverip");
  std::string port = MprpcApplication::GetConfig().Load("rpcserverport");
  uint16_t port_num = static_cast<uint16_t>(std::atoi(port.c_str()));

  InetAddress address(port_num, ip);
  server_ = std::make_unique<TcpServer>(&event_loop_, address, "RpcProvider");

  server_->setConnectionCallback(
      std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
  server_->setMessageCallback(
      std::bind(&RpcProvider::OnMessage,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3));

  MPRPC_LOG_INFO("rpc provider init success, ip=%s port=%d",
                 ip.c_str(),
                 port_num);
}

void RpcProvider::NotifyService(google::protobuf::Service *service)
{
  ServiceInfo serviceInfo;
  const google::protobuf::ServiceDescriptor *serviceDescriptor = service->GetDescriptor();
  std::string service_name = serviceDescriptor->name();
  int method_cnt = serviceDescriptor->method_count();
  for (int i = 0; i < method_cnt; i++)
  {
    const google::protobuf::MethodDescriptor *methodDescriptor = serviceDescriptor->method(i);
    std::string method_name = methodDescriptor->name();
    serviceInfo.method_map_.insert({method_name, methodDescriptor});
  }
  serviceInfo.service_ = service;
  service_map_.insert({service_name, serviceInfo});
  MPRPC_LOG_INFO("service notify success, service=%s method_count=%d",
                 service_name.c_str(),
                 method_cnt);
}

void RpcProvider::Run()
{
  std::string ip = MprpcApplication::GetConfig().Load("rpcserverip");
  std::string port = MprpcApplication::GetConfig().Load("rpcserverport");
  ZkClient zkcli;
  zkcli.Start();
  for (auto sp : service_map_)
  {
    std::string service_path = "/" + sp.first;
    zkcli.Create(service_path.c_str(), nullptr, 0);
    for (auto mp : sp.second.method_map_)
    {
      std::string method_path = service_path + "/" + mp.first;
      std::string method_data = ip + ":" + port;
      zkcli.Create(method_path.c_str(),
                   method_data.c_str(),
                   method_data.size(),
                   ZOO_EPHEMERAL);
    }
  }
  server_->setThreadNum(4);
  server_->start();
  MPRPC_LOG_INFO("rpc provider start, waiting for rpc request");
  event_loop_.loop();
}

void RpcProvider::OnConnection(const TcpConnection::TcpConnectionPtr &conn)
{
  if (!conn->connected())
  {
    conn->shutdown();
  }
}

void RpcProvider::OnMessage(const TcpConnection::TcpConnectionPtr &conn,
                            Buffer *buffer,
                            Timestamp receive_time)
{
  (void)receive_time;

  while (true)
  {
    if (buffer->readableBytes() < 4)
    {
      break;
    }
    const char *data = buffer->peek();
    uint32_t header_size = 0;
    std::memcpy(&header_size, data, 4);

    if (buffer->readableBytes() < 4 + header_size)
    {
      break;
    }

    std::string rpc_header_str(data + 4, header_size);
    mprpc::RpcHeader rpcHeader;
    if (!rpcHeader.ParseFromString(rpc_header_str))
    {
      MPRPC_LOG_ERR("parse rpc header error");
      conn->shutdown();
      break;
    }

    uint32_t args_size = rpcHeader.args_size();
    uint32_t total_size = 4 + header_size + args_size;
    if (buffer->readableBytes() < total_size)
    {
      break;
    }

    std::string service_name = rpcHeader.service_name();
    std::string method_name = rpcHeader.method_name();
    std::string args_str(data + 4 + header_size, args_size);

    auto service_it = service_map_.find(service_name);
    if (service_it == service_map_.end())
    {
      MPRPC_LOG_ERR("service not exist, service=%s", service_name.c_str());
      conn->shutdown();
      break;
    }

    auto method_it = service_it->second.method_map_.find(method_name);
    if (method_it == service_it->second.method_map_.end())
    {
      MPRPC_LOG_ERR("method not exist, service=%s method=%s",
                    service_name.c_str(),
                    method_name.c_str());
      conn->shutdown();
      break;
    }

    google::protobuf::Service *service = service_it->second.service_;
    const google::protobuf::MethodDescriptor *method = method_it->second;

    google::protobuf::Message *request =
        service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
      MPRPC_LOG_ERR("request parse error, service=%s method=%s",
                    service_name.c_str(),
                    method_name.c_str());
      delete request;
      conn->shutdown();
      break;
    }

    google::protobuf::Message *response =
        service->GetResponsePrototype(method).New();

    google::protobuf::Closure *done =
        new RpcProviderClosure(this, conn, request, response);

    service->CallMethod(method, nullptr, request, response, done);
    MPRPC_LOG_INFO("rpc request dispatch success, service=%s method=%s",
                   service_name.c_str(),
                   method_name.c_str());

    buffer->retrieve(total_size);
  }
}

void RpcProvider::SendRpcResponse(const TcpConnection::TcpConnectionPtr &conn,
                                  google::protobuf::Message *response)
{
  std::string response_str;
  if (!response->SerializeToString(&response_str))
  {
    MPRPC_LOG_ERR("serialize rpc response error");
    conn->shutdown();
    return;
  }

  conn->send(response_str);
}
