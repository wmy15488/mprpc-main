#pragma once
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string>

class InetAddress
{
    public:
    InetAddress(uint16_t port,std::string ip="127.0.0.0");
    InetAddress(sockaddr_in addr):addr_(addr){}
   const  sockaddr_in* getAddr() const;
   void   SetAddr(sockaddr_in addr);
    private:
    sockaddr_in addr_;
    
};
