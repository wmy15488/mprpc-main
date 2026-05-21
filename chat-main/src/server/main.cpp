#include "ChatServer.hpp"
#include <mymuduo/InetAddress.h>
#include <mymuduo/EventLoop.h>

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    uint16_t port = 8000;
    if (argc > 2)
    {
        std::cerr << "usage: " << argv[0] << " [port]" << std::endl;
        return 1;
    }

    if (argc == 2)
    {
        port = static_cast<uint16_t>(std::stoi(argv[1]));
    }

    EventLoop loop;
    InetAddress address(port, "127.0.0.1");
    ChatServer server(&loop, address, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}
