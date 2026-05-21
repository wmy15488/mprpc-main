#pragma once

#include "mprpcconfig.h"

class MprpcApplication {
public:
  static void Init(int argc, char** argv);

  static MprpcApplication& GetInstance();

  static MprpcConfig& GetConfig();

private:
  static MprpcConfig config_;
};
