#include "ChatClient.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    std::string ip = "127.0.0.1";
    unsigned short port = 8000;

    if (argc == 2)
    {
        port = static_cast<unsigned short>(std::stoi(argv[1]));
    }
    else if (argc == 3)
    {
        ip = argv[1];
        port = static_cast<unsigned short>(std::stoi(argv[2]));
    }
    else if (argc > 3)
    {
        std::cerr << "usage: " << argv[0] << " [port] | [ip port]" << std::endl;
        return 1;
    }

    ChatClient client;
    client.run(ip, port);
    return 0;
}
