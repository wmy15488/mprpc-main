#include<InetAddress.h>
#include<string.h>
InetAddress::InetAddress(uint16_t port,std::string ip)
{
   bzero(&addr_,sizeof(addr_));
   addr_.sin_family=AF_INET;
   addr_.sin_port=htons(port);
   addr_.sin_addr.s_addr=inet_addr(ip.c_str());
}

 const sockaddr_in* InetAddress::getAddr() const
{
    return &addr_;
}

void InetAddress::SetAddr(sockaddr_in addr)
{
addr_=addr;
}
