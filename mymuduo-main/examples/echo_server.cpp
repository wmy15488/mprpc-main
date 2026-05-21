#include <mymuduo/Buffer.h>
#include <mymuduo/EventLoop.h>
#include <mymuduo/InetAddress.h>
#include <mymuduo/TcpConnection.h>
#include <mymuduo/TcpServer.h>

#include <arpa/inet.h>
#include <cstdlib>
#include <string>

namespace {

// 把 InetAddress 转成 "ip:port" 字符串，主要用于日志打印。
std::string toIpPort(const InetAddress& addr) {
  const sockaddr_in* sa = addr.getAddr();
  char ip[INET_ADDRSTRLEN] = {0};
  ::inet_ntop(AF_INET, &sa->sin_addr, ip, sizeof(ip));
  return std::string(ip) + ":" + std::to_string(ntohs(sa->sin_port));
}

// 连接状态回调：新连接建立/连接断开时都会触发。
void onConnection(const TcpConnection::TcpConnectionPtr& conn) {
  if (conn->connected()) {
    // connected() 为 true 表示连接建立成功。
    LOG_INFO("new connection: %s -> %s, name=%s",
             toIpPort(conn->peerAddress()).c_str(),
             toIpPort(conn->localAddress()).c_str(),
             conn->name().c_str());
  } else {
    // connected() 为 false 通常表示连接已经断开。
    LOG_INFO("connection down: %s", conn->name().c_str());
  }
}

// 消息回调：客户端发来数据后触发。
// 这里实现最简单的 Echo 行为：收到什么，就回什么。
void onMessage(const TcpConnection::TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
  // 取出当前缓冲区中的全部可读数据。
  std::string msg = buf->retrieveAllAsString();
  // 把收到的数据原样发回给客户端。
  conn->send(msg);
}

}  // namespace

int main(int argc, char* argv[]) {
  // 默认监听 8888 端口，可通过命令行参数覆盖，例如：./echo_server 9000
  uint16_t port = 8888;
  if (argc > 1) {
    int parsed = std::atoi(argv[1]);
    if (parsed > 0 && parsed <= 65535) {
      port = static_cast<uint16_t>(parsed);
    }
  }

  // 创建主事件循环（Reactor 核心）。
  EventLoop loop;
  // 监听地址：0.0.0.0 表示监听本机所有网卡。
  InetAddress listenAddr(port, "0.0.0.0");
  // 创建 TCP 服务器对象，true 表示开启端口复用（reuseport）。
  TcpServer server(&loop, listenAddr, "EchoServer", true);

  // 注册连接回调与消息回调。
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);

  // 设置 IO 线程数量（除主 loop 外的 subloop 线程数）。
  server.setThreadNum(2);

  LOG_INFO("EchoServer start, listen on %u", port);
  // 启动服务（开始 listen）。
  server.start();
  // 进入事件循环，阻塞等待并处理网络事件。
  loop.loop();
  return 0;
}