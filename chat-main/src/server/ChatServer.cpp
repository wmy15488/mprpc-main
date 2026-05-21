#include "ChatServer.hpp"
#include <functional>
#include "ChatService.hpp"
using namespace std::placeholders;

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    _server.setMessageCallback(std::bind(&ChatServer::OnMessage, this, _1, _2, _3));
    _server.setConnectionCallback(std::bind(&ChatServer::OnConnection, this, _1));
    _server.setThreadNum(4);
}

void ChatServer::start()
{
    LOG_INFO("chatserver statr");
    _server.start();
}

void ChatServer::OnConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
    }
    else
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::OnMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string buff = buf->retrieveAllAsString();
    json js = json::parse(buff);
    auto msghandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msghandler(conn, js, time);
}
