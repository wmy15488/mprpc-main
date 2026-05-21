#pragma once

#include <semaphore.h>
#include <string>

#ifndef THREADED
#define THREADED
#endif
#include <zookeeper/zookeeper.h>

class ZkClient
{
public:
  ZkClient();
  ~ZkClient();

  // 启动 ZooKeeper 客户端连接，并等待会话建立成功
  void Start();

  // 创建指定路径的 znode 节点
  // state 一般传 0 表示持久节点，传 ZOO_EPHEMERAL 表示临时节点
  void Create(const char* path, const char* data, int datalen, int state = 0);

  // 获取指定 znode 节点保存的数据内容
  std::string GetData(const char* path);

private:
  // ZooKeeper 客户端句柄，由 zookeeper_init 返回
  zhandle_t* zk_handle_;
};
