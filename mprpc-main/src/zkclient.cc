#include "zkclient.h"

#include <cstring>
#include <iostream>

#include "mprpcapplication.h"

// 全局 watcher 观察器
// 当前教学版里主要关心会话连接成功事件，
// 用它来通知 Start() 连接已经建立完成
void global_watcher(zhandle_t* zh, int type, int state,
                    const char* path, void* watcherCtx)
{
  (void)path;
  (void)watcherCtx;

  if (type == ZOO_SESSION_EVENT && state == ZOO_CONNECTED_STATE)
  {
    // Start() 里把信号量通过上下文传给 ZooKeeper，
    // 这里在连接建立成功后唤醒等待线程
    sem_t* sem = const_cast<sem_t*>(
        static_cast<const sem_t*>(zoo_get_context(zh)));
    sem_post(sem);
  }
}

ZkClient::ZkClient()
    : zk_handle_(nullptr)
{
}

ZkClient::~ZkClient()
{
  if (zk_handle_ != nullptr)
  {
    zookeeper_close(zk_handle_);
  }
}

void ZkClient::Start()
{
  std::string ip = MprpcApplication::GetConfig().Load("zookeeperip");
  std::string port = MprpcApplication::GetConfig().Load("zookeeperport");
  std::string connstr = ip + ":" + port;

  // zookeeper_init 是异步连接的，所以这里用信号量阻塞等待，
  // 直到 watcher 通知连接成功后再继续往下执行
  sem_t sem;
  sem_init(&sem, 0, 0);

  zk_handle_ = zookeeper_init(connstr.c_str(),
                              global_watcher,
                              30000,
                              nullptr,
                              &sem,
                              0);
  if (zk_handle_ == nullptr)
  {
    std::cout << "zookeeper_init error!" << std::endl;
    exit(EXIT_FAILURE);
  }

  // 等待 watcher 回调通知 ZooKeeper 会话连接成功
  sem_wait(&sem);
  sem_destroy(&sem);
}

void ZkClient::Create(const char* path, const char* data, int datalen, int state)
{
  char path_buffer[128];
  int bufferlen = sizeof(path_buffer);

  // 先判断当前节点是否已经存在，只有不存在时才创建
  // 这样可以避免重复注册时报错
  int flag = zoo_exists(zk_handle_, path, 0, nullptr);
  if (flag == ZNONODE)
  {
    flag = zoo_create(zk_handle_,
                      path,
                      data,
                      datalen,
                      &ZOO_OPEN_ACL_UNSAFE,
                      state,
                      path_buffer,
                      bufferlen);
    if (flag != ZOK)
    {
      std::cout << "znode create error... path:" << path << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

std::string ZkClient::GetData(const char* path)
{
  char buffer[64];
  int bufferlen = sizeof(buffer);

  // 读取指定节点保存的数据，后面一般会用来获取 provider 的 ip:port
  int flag = zoo_get(zk_handle_, path, 0, buffer, &bufferlen, nullptr);
  if (flag != ZOK)
  {
    return "";
  }

  return std::string(buffer, bufferlen);
}
