#include "logger.h"

#include <ctime>
#include <fstream>

MprpcLogger &MprpcLogger::GetInstance()
{
  static MprpcLogger logger;
  return logger;
}

void MprpcLogger::SetLogLevel(MprpcLogLevel level)
{
  log_level_ = level;
}

void MprpcLogger::Log(const std::string &msg)
{
  lock_queue_.Push(msg);
}

MprpcLogger::MprpcLogger()
    : log_level_(MPRPC_INFO),
      is_exit_(false),
      write_thread_(&MprpcLogger::FlushLogThread, this)
{
}

MprpcLogger::~MprpcLogger()
{
  is_exit_ = true;
  lock_queue_.Close();
  if (write_thread_.joinable())
  {
    write_thread_.join();
  }
}

void MprpcLogger::FlushLogThread()
{
  while (true)
  {
    std::string msg = lock_queue_.Pop();
    if (msg.empty())
    {
      if (is_exit_)
      {
        break;
      }
      continue;
    }
    time_t now = time(nullptr);
    tm *nowtm = localtime(&now);
    char file_name[128] = {0};
    snprintf(file_name,
             sizeof(file_name),
             "%d-%02d-%02d-log.txt",
             nowtm->tm_year + 1900,
             nowtm->tm_mon + 1,
             nowtm->tm_mday);
    std::ofstream ofs(file_name, std::ios::app);
    ofs << msg << std::endl;
    ofs.close();
  }
}
