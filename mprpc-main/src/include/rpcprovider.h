#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <google/protobuf/service.h>
#include <mymuduo/EventLoop.h>
#include <mymuduo/TcpConnection.h>
#include <mymuduo/TcpServer.h>

class Buffer;
class Timestamp;

class RpcProvider {
public:
  RpcProvider();

  void NotifyService(google::protobuf::Service* service);

  void Run();

  void OnConnection(const TcpConnection::TcpConnectionPtr& conn);

  void OnMessage(const TcpConnection::TcpConnectionPtr& conn,
                 Buffer* buffer,
                 Timestamp receive_time);

  void SendRpcResponse(const TcpConnection::TcpConnectionPtr& conn,
                       google::protobuf::Message* response);

private:
  struct ServiceInfo {
    google::protobuf::Service* service_;
    std::unordered_map<std::string,
                       const google::protobuf::MethodDescriptor*> method_map_;
  };

private:
  std::unordered_map<std::string, ServiceInfo> service_map_;
  EventLoop event_loop_;
  std::unique_ptr<TcpServer> server_;
};
