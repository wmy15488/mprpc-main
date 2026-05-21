#pragma once

#include <cstdio>
#include <string>
#include <thread>
#include <ctime>

#include "lockqueue.h"

enum MprpcLogLevel
{
  MPRPC_INFO,
  MPRPC_ERROR
};

class MprpcLogger
{
public:
  static MprpcLogger &GetInstance();

  void SetLogLevel(MprpcLogLevel level);

  void Log(const std::string &msg);

  ~MprpcLogger();

private:
  MprpcLogger();
  MprpcLogger(const MprpcLogger &) = delete;
  MprpcLogger(MprpcLogger &&) = delete;
  MprpcLogger &operator=(const MprpcLogger &) = delete;
  MprpcLogger &operator=(MprpcLogger &&) = delete;
  void FlushLogThread();

private:
  MprpcLogLevel log_level_;
  LockQueue<std::string> lock_queue_;
  std::thread write_thread_;
  bool is_exit_;
};

#define MPRPC_LOG_INFO(logmsgformat, ...) \
  do                                                               \
  {                                                                \
    MprpcLogger &logger = MprpcLogger::GetInstance();              \
    logger.SetLogLevel(MPRPC_INFO);                                \
    char log_buf[1024] = {0};                                \
    std::snprintf(log_buf, sizeof(log_buf),                  \
                  logmsgformat, ##__VA_ARGS__);              \
    std::time_t now = std::time(nullptr);                    \
    std::tm *nowtm = std::localtime(&now);                   \
    char msg_buf[1024] = {0};                                \
    std::snprintf(msg_buf, sizeof(msg_buf),                  \
                  "[INFO] %d-%02d-%02d %02d:%02d:%02d : %s", \
                  nowtm->tm_year + 1900,                     \
                  nowtm->tm_mon + 1,                         \
                  nowtm->tm_mday,                            \
                  nowtm->tm_hour,                            \
                  nowtm->tm_min,                             \
                  nowtm->tm_sec,                             \
                  log_buf);                                  \
    logger.Log(msg_buf);                                           \
  } while (0)

#define MPRPC_LOG_ERR(logmsgformat, ...) \
  do                                                               \
  {                                                                \
    MprpcLogger &logger = MprpcLogger::GetInstance();              \
    logger.SetLogLevel(MPRPC_ERROR);                               \
    char log_buf[1024] = {0};                                 \
    std::snprintf(log_buf, sizeof(log_buf),                   \
                  logmsgformat, ##__VA_ARGS__);               \
    std::time_t now = std::time(nullptr);                     \
    std::tm *nowtm = std::localtime(&now);                    \
    char msg_buf[1024] = {0};                                 \
    std::snprintf(msg_buf, sizeof(msg_buf),                   \
                  "[ERROR] %d-%02d-%02d %02d:%02d:%02d : %s", \
                  nowtm->tm_year + 1900,                      \
                  nowtm->tm_mon + 1,                          \
                  nowtm->tm_mday,                             \
                  nowtm->tm_hour,                             \
                  nowtm->tm_min,                              \
                  nowtm->tm_sec,                              \
                  log_buf);                                   \
    logger.Log(msg_buf);                                           \
  } while (0)
