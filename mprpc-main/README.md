# mprpc

一个基于 `C++17 + Muduo + Protobuf + ZooKeeper` 实现的轻量级 RPC 通信框架。  
项目从零搭建并完整实现了 **客户端远程调用、服务端请求分发、异步日志、ZooKeeper 服务注册与发现** 等核心能力，目标是用工程方式理解一条 RPC 调用链如何真正落地。

## 项目概览

这个项目并不是只停留在“调用一次远程函数”的 Demo 层面，而是把 RPC 框架中最关键的几层都自己实现了一遍：

- 使用 `protobuf` 定义服务接口，生成 `Stub / Service`
- 自定义 RPC 请求协议：`header_size + rpc_header + args`
- 封装 `MprpcChannel`，将本地调用转换为远程请求
- 封装 `RpcProvider`，完成请求拆包、服务查找、方法分发
- 实现 `MprpcController`，区分 RPC 过程错误与业务错误
- 实现异步日志组件，支持后台线程写日志
- 封装 `ZkClient`，完成服务注册与服务发现
- 从“单机直连版”演进到“ZooKeeper 注册发现版”

## 技术栈

- `C++17`
- `Muduo`
- `Protobuf`
- `ZooKeeper C API`
- `CMake`

## 核心能力

### 1. 本地调用风格的远程调用

通过 protobuf 生成的 `Stub` 发起调用，业务层调用方式与普通本地函数接近：

- 业务代码通过 `UserServiceRpc_Stub` 发起请求
- 底层进入 `MprpcChannel::CallMethod()`
- 自动完成参数序列化、组包、网络发送、响应反序列化

### 2. 服务端动态分发

服务端不写死某个方法，而是通过 protobuf 描述信息完成动态分发：

- 根据 `service_name + method_name` 找到目标服务
- 动态创建 `request / response`
- 调用 `service->CallMethod(...)`

这使框架具备了通用 RPC 分发能力，而不只针对某一个业务接口。

### 3. ZooKeeper 服务注册与发现

框架已完成从单机写死地址到注册中心发现地址的升级：

- Provider 启动后，把服务和方法注册到 ZooKeeper
- Caller 调用前，从 ZooKeeper 查询目标服务地址
- 方法节点使用临时节点，provider 下线后地址可自动失效

### 4. 异步日志模块

实现了基础异步日志器：

- `LockQueue` 实现线程安全阻塞队列
- `MprpcLogger` 负责日志投递
- 后台线程异步写入日志文件

日志文件按日期命名，写入当前运行目录。

## 项目亮点

- 从零实现了一版可运行的 RPC 框架，而不是只使用现成框架
- 既实现了单机版调用，也实现了 ZooKeeper 版服务发现
- 框架分层清晰，通信层、控制层、服务层、注册发现层职责明确
- 对 protobuf 的 `Stub / Service / CallMethod / Descriptor` 机制有完整实践
- 对粘包半包、请求头设计、服务注册发现流程有完整落地

## RPC 调用流程图

```text
服务启动阶段

callee
  -> MprpcApplication 读取配置
  -> RpcProvider 注册本地 Service
  -> ZkClient 连接 ZooKeeper
  -> 在 ZooKeeper 中注册：
       /ServiceName
       /ServiceName/MethodName -> ip:port
  -> 启动 Muduo TcpServer，等待请求


一次 RPC 调用阶段

caller
  -> 业务代码调用 Stub.Login(...)
  -> MprpcChannel::CallMethod()
  -> 序列化 request，组装 rpc_header + args
  -> ZkClient 从 ZooKeeper 查询：
       /ServiceName/MethodName -> ip:port
  -> 建立 TCP 连接并发送请求
  -> provider 收到请求并拆包
  -> 根据 service_name / method_name 找到目标服务
  -> service->CallMethod(...) 分发到具体业务方法
  -> 业务方法填充 response
  -> provider 序列化 response 返回 caller
  -> caller 反序列化 response，得到最终结果
```

## 架构说明

### 调用方流程

1. 业务代码通过 protobuf 生成的 `Stub` 发起调用
2. `Stub` 底层进入 `MprpcChannel::CallMethod()`
3. `MprpcChannel` 序列化参数并构造自定义 RPC 请求
4. 从 ZooKeeper 查询 `service/method` 对应的 provider 地址
5. 建立 TCP 连接，发送请求并接收响应

### 服务端流程

1. 业务服务继承 protobuf 生成的 `xxxServiceRpc`
2. 启动时通过 `RpcProvider::NotifyService()` 注册服务对象
3. `RpcProvider::Run()` 启动 TCP 服务并将服务注册到 ZooKeeper
4. 收到请求后解析包头、查找服务、分发方法
5. 调用业务方法并将响应返回给客户端

## 目录结构

```text
mprpc
├── config/                 # 配置文件
├── example/                # 示例调用方与服务提供方
│   ├── caller/
│   ├── callee/
│   └── user.proto
├── proto/                  # 框架内部协议定义
├── src/
│   ├── include/            # 框架头文件
│   ├── mprpcapplication.cc # 框架初始化
│   ├── mprpcconfig.cc      # 配置加载
│   ├── mprpccontroller.cc  # RPC 调用错误管理
│   ├── mprpcchannel.cc     # 客户端调用通道
│   ├── rpcprovider.cc      # 服务端 provider
│   ├── logger.cc           # 异步日志
│   └── zkclient.cc         # ZooKeeper 客户端封装
├── CMakeLists.txt
└── README.md
```

## 当前已完成

- `MprpcApplication`
- `MprpcConfig`
- `MprpcController`
- `MprpcChannel`
- `RpcProvider`
- `MprpcLogger`
- `ZkClient`
- `example/caller` 与 `example/callee`
- `Login` 的单机版调用验证
- `Login` 的 ZooKeeper 服务发现版调用验证

## 构建方式

```bash
cmake -S . -B build
cmake --build build
```

构建完成后主要产物：

- `build/lib/libmprpc.a`
- `build/bin/caller`
- `build/bin/callee`

## 配置文件

当前示例配置文件：

- [config/mprpc.conf](config/mprpc.conf)

示例内容：

```conf
rpcserverip=127.0.0.1
rpcserverport=8000
zookeeperip=127.0.0.1
zookeeperport=2181
```

含义：

- `rpcserverip / rpcserverport`
  - Provider 监听并注册到 ZooKeeper 的地址
- `zookeeperip / zookeeperport`
  - ZooKeeper 服务地址

## 运行方式

### 1. 确认 ZooKeeper 已启动

```bash
echo ruok | nc 127.0.0.1 2181
```

若返回：

```text
imok
```

说明 ZooKeeper 运行正常。

### 2. 启动服务提供方

```bash
./build/bin/callee -c ./config/mprpc.conf
```

### 3. 启动服务调用方

```bash
./build/bin/caller -c ./config/mprpc.conf
```

调用成功后：

- 终端会输出 RPC 返回结果
- 当前运行目录下会生成按日期命名的日志文件

## 示例服务

当前示例接口定义在 [example/user.proto](example/user.proto) 中，包含两个方法：

- `Login`
- `Register`

目前已完成并验证：

- `Login` 单机直连调用
- `Login` 基于 ZooKeeper 的服务注册与发现调用

## 这个项目体现的能力

这个项目重点体现的是对 C++ 后端基础能力和框架实现能力的理解，包括：

- 网络编程与 TCP 通信
- Protobuf 在 RPC 场景中的使用
- 服务端请求分发与动态方法调用
- RPC 错误控制与业务结果分离
- ZooKeeper 注册发现机制
- 从单机版到注册中心版的架构演进

## 后续可扩展方向

- 多 provider 实例与负载均衡
- ZooKeeper watcher 监听节点变化
- 长连接与连接复用
- 更完整的响应协议与超时控制
- 更完善的日志滚动与日志级别管理

---

如果你正在阅读这个项目，欢迎交流 C++ 后端、网络编程、RPC 框架实现、ZooKeeper 服务发现等相关问题。
