#pragma once

#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "nocopyable.h"

#include <atomic>
#include <map>
#include <memory>
#include <string>

class EventLoop;

class TcpServer : public nocopyable {
 public:
  using ThreadInitCallback = EventLoopThreadPool::ThreadInitCallback;
  using ConnectionCallback = TcpConnection::ConnectionCallback;
  using MessageCallback = TcpConnection::MessageCallback;
  using WriteCompleteCallback = TcpConnection::WriteCompleteCallback;
  using TcpConnectionPtr = TcpConnection::TcpConnectionPtr;
  using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

  TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& nameArg,
            bool reuseport = false);
  ~TcpServer();

  const std::string& name() const { return name_; }
  EventLoop* getLoop() const { return loop_; }

  void setThreadNum(int numThreads);
  void start();

  void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
  void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
  }

 private:
  static InetAddress getLocalAddr(int sockfd);
  void newConnection(int sockfd, const InetAddress& peerAddr);
  void removeConnection(const TcpConnectionPtr& conn);
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> threadPool_;

  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  ThreadInitCallback threadInitCallback_;

  std::atomic_int started_;
  int nextConnId_;
  ConnectionMap connections_;
};
