#pragma once

#include <mymuduo/TcpServer.h>
#include <mymuduo/EventLoop.h>
#include <mymuduo/InetAddress.h>
#include <mymuduo/TcpConnection.h>
#include <string>

using TcpConnectionPtr=TcpConnection::TcpConnectionPtr;


class ChatServer
{
public:
    ChatServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg);

    void start();

private:
    TcpServer _server;
    EventLoop* _loop;

    void OnConnection(const TcpConnectionPtr&);
    void OnMessage(const TcpConnectionPtr&,Buffer*,Timestamp);
};


