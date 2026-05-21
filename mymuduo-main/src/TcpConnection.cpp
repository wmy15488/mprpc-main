#include "TcpConnection.h"

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

#include <functional>

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &name,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      reading_(true)
{
  channel_->SetreadEventback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
  channel_->SetwriteEventback(std::bind(&TcpConnection::handleWrite, this));
  channel_->SetcloseEventback(std::bind(&TcpConnection::handleClose, this));
  channel_->SeterrorEventback(std::bind(&TcpConnection::handleError, this));
  socket_->setKeepAlive(true);
  socket_->setTcpNoDelay(true);
}

TcpConnection::~TcpConnection() = default;

void TcpConnection::send(const std::string &message)
{
  // 统一走底层发送接口。
  send(message.data(), message.size());
}

void TcpConnection::send(const void *data, size_t len)
{
  // 线程安全发送入口。
  if (state_ == kConnected)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(data, len);
    }
    else
    {
      std::string message(static_cast<const char *>(data), len);
      loop_->runInLoop([this, message]()
                       { sendInLoop(message.data(), message.size()); });
    }
  }
}

void TcpConnection::shutdown()
{
  // 发起半关闭。
  if (state_ == kConnected)
  {
    setState(kDisconnecting);
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::connectEstablished()
{
  // 建连后的状态与回调处理。
  setState(kConnected);
  channel_->tie(shared_from_this());
  channel_->ableread();
  if (connectionCallback_)
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
  // 销毁前清理 channel 并回调。
  if (state_ == kConnected)
  {
    setState(kDisconnected);
  }
  channel_->disableAll();
  channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
  // 读取数据并分发消息/关闭/错误。
  int savedError = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedError);
  if (n > 0)
  {
    if (messageCallback_)
    {
      messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
  }
  else if (n == 0)
  {
    handleClose();
  }
  else
  {
    errno = savedError;
    handleError();
  }
}

void TcpConnection::handleWrite()
{
  // 处理可写事件，flush outputBuffer_。
  if (channel_->iswrite())
  {
    ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
    if (n > 0)
    {
      outputBuffer_.retrieve(static_cast<size_t>(n));
      if (outputBuffer_.readableBytes() == 0)
      {
        channel_->disablewrite();
        if (writeCompleteCallback_)
        {
          writeCompleteCallback_(shared_from_this());
        }
        if (state_ == kDisconnecting)
        {
          shutdownInLoop();
        }
      }
    }
    else
    {
      LOG_ERORR("TcpConnection::handleWrite");
    }
  }
}

void TcpConnection::handleClose()
{
  // 处理连接关闭并触发回调。
  setState(kDisconnected);
  TcpConnectionPtr guardThis(shared_from_this());
  if (connectionCallback_)
  {
    connectionCallback_(guardThis);
  }
  if (closeCallback_)
  {
    closeCallback_(guardThis);
  }
}

void TcpConnection::handleError()
{
  // 获取 SO_ERROR 并记录。

  int err = 0;
  socklen_t len = static_cast<socklen_t>(sizeof err);
  if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &err, &len) < 0)
  {
    err = errno;
  }
  LOG_ERORR("TcpConnection::handleError name:%s errno:%d", name_.c_str(), err);
}

void TcpConnection::sendInLoop(const void *data, size_t len)
{
  // loop 线程内发送（直写+缓冲）。
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;

  if (state_ == kDisconnected)
    return;

  if (channel_->iswrite() == false && outputBuffer_.readableBytes() == 0)
  {
    nwrote = ::write(channel_->fd(), data, len);
    if (nwrote >= 0)
    {
      remaining = len - static_cast<size_t>(nwrote);
      if (remaining == 0 && writeCompleteCallback_)
      {
        writeCompleteCallback_(shared_from_this());
      }
    }
    else
    {
      nwrote = 0;
      if (errno != EWOULDBLOCK)
      {
        LOG_ERORR("TcpConnection::sendInLoop");
        if (errno == EPIPE || errno == ECONNRESET)
        {
          faultError = true;
        }
      }
    }
  }

  if (!faultError && remaining > 0)
  {
    outputBuffer_.append(static_cast<const char *>(data) + nwrote, remaining);
    if (!channel_->iswrite())
    {
      channel_->ablewrite();
    }
  }
}

void TcpConnection::shutdownInLoop()
{
  // outputBuffer_ 清空后 shutdownWrite。
  if (!channel_->iswrite())
  {
    socket_->shutdownWrite();
  }
}
