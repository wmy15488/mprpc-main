#include "userservice.h"

#include "logger.h"

using namespace fixbug;

bool UserService::Login(const std::string &name, const std::string &pwd)
{
  MPRPC_LOG_INFO("doing local service: Login, name=%s pwd=%s",
                 name.c_str(),
                 pwd.c_str());
  //模拟数据查表操作
  if (name == "zhang san" && pwd == "123456")
  {
    return true;
  }

  return false;
}

bool UserService::Register(uint32_t id,
                           const std::string &name,
                           const std::string &pwd)
{
  MPRPC_LOG_INFO("doing local service: Register, id=%u name=%s pwd=%s",
                 id,
                 name.c_str(),
                 pwd.c_str());
  
  return true;
}

void UserService::Login(::google::protobuf::RpcController *controller,
                        const LoginRequest *request,
                        LoginResponse *response,
                        ::google::protobuf::Closure *done)
{
 (void)controller;

 std::string name = request->name();
 std::string pwd = request->pwd();
 
 bool login_result = Login(name, pwd);

 response->mutable_result()->set_errcode(0);
 response->mutable_result()->set_errmsg("");
 response->set_success(login_result);

 done->Run();
}

void UserService::Register(::google::protobuf::RpcController *controller,
                           const RegisterRequest *request,
                           RegisterResponse *response,
                           ::google::protobuf::Closure *done)
{
 (void)controller;

 uint32_t id = request->id();
 std::string name = request->name();
 std::string pwd = request->pwd();

 bool register_result = Register(id, name, pwd);

 response->mutable_result()->set_errcode(0);
 response->mutable_result()->set_errmsg("");
 response->set_success(register_result);

 done->Run();
}
