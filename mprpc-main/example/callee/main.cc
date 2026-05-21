#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "userservice.h"

int main(int argc, char** argv) {
  MprpcApplication::Init(argc, argv);

  RpcProvider provider;
  provider.NotifyService(new UserService());
  provider.Run();

  return 0;
}
