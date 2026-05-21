#include "TcpServer.h"

#include "EventLoop.h"

#include <strings.h>
#include <functional>
#include <sys/socket.h>

InetAddress TcpServer::getLocalAddr(int sockfd)
{
  sockaddr_in localAddr;
  bzero(&localAddr, sizeof(localAddr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(localAddr));
  if (::getsockname(sockfd, reinterpret_cast<sockaddr *>(&localAddr), &addrlen) < 0)
  {
    LOG_ERORR("TcpServer::getLocalAddr getsockname error");
  }
  return InetAddress(localAddr);
}

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const std::string &nameArg,
                     bool reuseport)
    : loop_(loop),
      name_(nameArg),
      acceptor_(new Acceptor(loop, listenAddr, reuseport)),
      threadPool_(new EventLoopThreadPool(loop, nameArg)),
      started_(0),
      nextConnId_(1)
{
  // 绑定 acceptor 的新连接回调到 newConnection。
  acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
  // 析构时清理连接表，确保所有连接都能安全退出事件循环。
  for(auto &it:connections_)
  {
    TcpConnectionPtr conn =it.second;
    it.second.reset();
    conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed,conn));
  }
}

void TcpServer::setThreadNum(int numThreads)
{
  // 设置 IO 线程池中的 subloop 线程数量。
  threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
  // 启动线程池，并在 base loop 中开始 listen。
  if (started_++ == 0)
  {
    threadPool_->start(threadInitCallback_);
    loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
  }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
  // 为新连接选择 loop、构造 TcpConnection、设置回调并建立连接。
  EventLoop *ioLoop = threadPool_->getNextLoop();
  std::string connName = name_ + "#" + std::to_string(nextConnId_++);

  InetAddress localAddr(TcpServer::getLocalAddr(sockfd));
  TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
  connections_[connName] = conn;

  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
  // 对外的移除入口：切回 base loop 做统一删除。
  loop_->runInLoop(
      std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
  // 从 connections_ 擦除连接，并触发 connectDestroyed。
  connections_.erase(conn->name());
  EventLoop*ioloop=conn->getLoop();
  ioloop->queueInLoop(std::bind(&TcpConnection::connectDestroyed,conn));
}
