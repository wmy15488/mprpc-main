#pragma once

#include <cstdint>
#include <string>

#include "user.pb.h"

using namespace fixbug;

class UserService : public UserServiceRpc {
public:
  bool Login(const std::string& name, const std::string& pwd);

  bool Register(uint32_t id, const std::string& name, const std::string& pwd);

  void Login(::google::protobuf::RpcController* controller,
             const LoginRequest* request,
             LoginResponse* response,
             ::google::protobuf::Closure* done) override;

  void Register(::google::protobuf::RpcController* controller,
                const RegisterRequest* request,
                RegisterResponse* response,
                ::google::protobuf::Closure* done) override;
};
