#include "Acceptor.h"

#include "EventLoop.h"

int Acceptor::createNonblocking() {
  // 创建非阻塞、关闭时自动关闭(fd)的 TCP 监听套接字。
  int sockfd=::socket(AF_INET,SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK,IPPROTO_TCP);
  return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(createNonblocking()),
      acceptChannel_(loop, acceptSocket_.fd()),
      listening_(false) {
  acceptSocket_.setKeepAlive(true);
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(reuseport);
  acceptSocket_.bindAddress(listenAddr);
  acceptChannel_.SetreadEventback(std::bind(&Acceptor::handleRead,this));
  // 绑定地址、设置端口复用参数，并将读事件回调绑定到 handleRead。
}

Acceptor::~Acceptor() = default;

void Acceptor::setNewConnectionCallback(const NewConnectionCallback& cb) {
  newConnectionCallback_ =std::move(cb);
}

bool Acceptor::listening() const { return listening_; }

void Acceptor::listen() {
  listening_ = true;
  // 启动 listen，并使 acceptChannel_ 关注读事件。
  acceptSocket_.listen();
  acceptChannel_.ableread();
}

void Acceptor::handleRead() {
  // 接收新连接，并调用 newConnectionCallback_ 交给上层处理。
  InetAddress peeraddr(0);
  int connfd=acceptSocket_.accept(&peeraddr);
  if(connfd>=0)
  {
    if(newConnectionCallback_ )
  newConnectionCallback_(connfd,peeraddr);
   else ::close(connfd);
  }
  else
  LOG_ERORR("accept err");

}
