#include "Socket.h"
#include "Logger.h"

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>

Socket::Socket(int sockfd) : sockfd_(sockfd) {}

Socket::~Socket() {
  // 关闭套接字 fd。
  ::close(sockfd_);
}

int Socket::fd() const { return sockfd_; }

void Socket::bindAddress(const InetAddress& localaddr) {
  // 调用 bind 将 sockfd_ 绑定到 localaddr。
  if (::bind(sockfd_,
             reinterpret_cast<const sockaddr*>(localaddr.getAddr()),
             static_cast<socklen_t>(sizeof(sockaddr_in))) < 0) {
    LOG_FATAL("bind error, sockfd:%d errno:%d", sockfd_, errno);
  }
}

void Socket::listen() {
  // 调用 listen 开始监听连接。
  if (::listen(sockfd_, SOMAXCONN) < 0) {
    LOG_FATAL("listen error, sockfd:%d errno:%d", sockfd_, errno);
  }
}

int Socket::accept(InetAddress* peeraddr) {
  // 调用 accept4 接收连接并写回对端地址。
  sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  socklen_t len = static_cast<socklen_t>(sizeof(addr));

  int connfd = ::accept4(
      sockfd_, reinterpret_cast<sockaddr*>(&addr), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd >= 0 && peeraddr != nullptr) {
    peeraddr->SetAddr(addr);
  }
  return connfd;
}

void Socket::shutdownWrite() {
  // 关闭写方向（SHUT_WR）。
  if (::shutdown(sockfd_, SHUT_WR) < 0) {
    LOG_ERORR("shutdownWrite error, sockfd:%d errno:%d", sockfd_, errno);
  }
}

void Socket::setTcpNoDelay(bool on) {
  // 设置 TCP_NODELAY。
  int optval = on ? 1 : 0;
  if (::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) < 0) {
    LOG_ERORR("setTcpNoDelay error, sockfd:%d errno:%d", sockfd_, errno);
  }
}

void Socket::setReuseAddr(bool on) {
  // 设置 SO_REUSEADDR。
  int optval = on ? 1 : 0;
  if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
    LOG_ERORR("setReuseAddr error, sockfd:%d errno:%d", sockfd_, errno);
  }
}

void Socket::setReusePort(bool on) {
  // 设置 SO_REUSEPORT。
  int optval = on ? 1 : 0;
  if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
    LOG_ERORR("setReusePort error, sockfd:%d errno:%d", sockfd_, errno);
  }
}

void Socket::setKeepAlive(bool on) {
  // 设置 SO_KEEPALIVE。
  int optval = on ? 1 : 0;
  if (::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
    LOG_ERORR("setKeepAlive error, sockfd:%d errno:%d", sockfd_, errno);
  }
}
