#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
MprpcConfig MprpcApplication::config_;

void MprpcApplication::Init(int argc, char **argv)
{
  if (argc < 3)
  {
    std::cout << "input format: command -c <configfile>" << std::endl;
    exit(EXIT_FAILURE);
  }
  int c = 0;
  while ((c = getopt(argc, argv, "c:")) != -1)
  {
    switch (c)
    {
    case 'c':
      config_.LoadConfigFile(optarg);
      break;

    default:
      std::cout << "input format: command -c <configfile>" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

MprpcApplication &MprpcApplication::GetInstance()
{
  static MprpcApplication app;
  return app;
}

MprpcConfig &MprpcApplication::GetConfig()
{
  return config_;
}
