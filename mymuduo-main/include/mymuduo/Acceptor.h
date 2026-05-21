#pragma once

#include "Channel.h"
#include "InetAddress.h"
#include "Socket.h"
#include "nocopyable.h"

#include <functional>

class EventLoop;

class Acceptor : public nocopyable {
 public:
  using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback& cb);
  bool listening() const;
  void listen();

 private:
  static int createNonblocking();
  void handleRead();

  EventLoop* loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listening_;
};
