#include <iostream>

#include "mprpcapplication.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
#include "user.pb.h"

using namespace fixbug;

int main(int argc, char** argv)
{
  MprpcApplication::Init(argc, argv);

  UserServiceRpc_Stub stub(new MprpcChannel());
  LoginRequest request;
  request.set_name("zhang san");
  request.set_pwd("123456");

  LoginResponse response;
  MprpcController controller;

  stub.Login(&controller, &request, &response, nullptr);

  if (controller.Failed())
  {
    std::cout << "rpc login failed: " << controller.ErrorText() << std::endl;
    return 0;
  }

  if (response.result().errcode() == 0)
  {
    std::cout << "rpc login response success: "
              << response.success() << std::endl;
  }
  else
  {
    std::cout << "rpc login response error: "
              << response.result().errmsg() << std::endl;
  }

  return 0;
}
