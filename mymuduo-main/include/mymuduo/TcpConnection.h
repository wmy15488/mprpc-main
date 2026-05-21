#pragma once

#include "Buffer.h"
#include "InetAddress.h"
#include "nocopyable.h"

#include <functional>
#include <memory>
#include <string>

class Channel;
class EventLoop;
class Socket;
class Timestamp;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>, public nocopyable {
 public:
  using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
  using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
  using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;
  using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
  using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

  TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& localAddress() const { return localAddr_; }
  const InetAddress& peerAddress() const { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }
  bool disconnected() const { return state_ == kDisconnected; }

  void send(const std::string& message);
  void send(const void* data, size_t len);
  void shutdown();

  void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
  void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

  void connectEstablished();
  void connectDestroyed();

 private:
  enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

  void setState(StateE state) { state_ = state; }

  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();

  void sendInLoop(const void* data, size_t len);
  void shutdownInLoop();



  EventLoop* loop_;
  const std::string name_;
  StateE state_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  bool reading_;

  Buffer inputBuffer_;
  Buffer outputBuffer_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  CloseCallback closeCallback_;
};
