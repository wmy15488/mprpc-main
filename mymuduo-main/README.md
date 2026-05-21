# mymuduo

> 基于 Muduo 网络库思想的学习型实现，采用 `epoll + Reactor + One Loop Per Thread` 构建事件驱动的高性能 TCP 网络库。

## 项目简介

这个项目是对 Muduo 核心设计的拆解与复现，目标是通过“自己实现一遍”的方式理解高并发网络编程中的关键抽象。

当前实现聚焦于：

- Reactor 事件驱动模型
- `epoll` I/O 多路复用
- 非阻塞 I/O
- 线程封装与线程池协作
- TCP 连接生命周期管理

## 当前实现的核心模块

- `EventLoop`：事件循环与任务调度
- `Channel`：fd 与事件回调绑定
- `Poller` / `EPollPoller`：`epoll` 封装
- `Buffer`：收发缓冲区
- `TcpConnection`：连接对象与读写流程
- `Acceptor`：监听与接收新连接
- `TcpServer`：服务端主抽象
- `EventLoopThread` / `EventLoopThreadPool`：One Loop Per Thread 支撑
- `Thread` / `CurrentThread`：线程工具封装
- `Socket` / `InetAddress`：网络地址与套接字封装
- `Logger` / `Timestamp`：日志与时间戳工具

## 架构简图

```text
            +-----------------------+
            |       TcpServer       |
            +----------+------------+
                       |
                       v
               +-------+--------+
               |    Acceptor    |
               +-------+--------+
                       |
                       v
+----------------------+----------------------+
|                  EventLoop                  |
|     +---------------+------------------+    |
|     |              Poller              |    |
|     |     (EPollPoller / epoll_wait)   |    |
|     +---------------+------------------+    |
|                     |                       |
|                     v                       |
|                  Channel                    |
+---------------------------------------------+
                       |
                       v
                TcpConnection
```

## 目录结构

- `include/mymuduo/`：对外暴露的公共头文件
- `src/`：库实现
- `examples/`：示例程序
- `cmake/`：CMake 包配置模板
- `build/`：构建产物目录

现在项目已经按可复用库的结构整理完成，适合安装后被其它项目直接引用。

## 构建

> 以 Linux 环境为例。

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -j
```

构建完成后，`build/` 目录下会生成：

- `libmymuduo.so`
- `echo_server`

## 安装

安装到系统默认前缀 `/usr/local`：

```bash
cd build
sudo cmake --install .
```

安装后默认文件位置为：

- `/usr/local/include/mymuduo/`
- `/usr/local/lib/libmymuduo.so`
- `/usr/local/lib/cmake/mymuduo/`

如果要安装到自定义前缀，可以在配置阶段指定：

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/your/install/prefix
```

## 外部项目使用

### 头文件引用

```cpp
#include <mymuduo/EventLoop.h>
#include <mymuduo/InetAddress.h>
#include <mymuduo/TcpServer.h>
```

### CMake 使用方式

推荐使用标准 CMake 包方式：

```cmake
find_package(mymuduo REQUIRED)

add_executable(your_app main.cpp)
target_link_libraries(your_app PRIVATE mymuduo::mymuduo)
```

这种方式下，`mymuduo` 自己的头文件目录和线程依赖会通过导出的 CMake target 自动传递给外部项目，不需要在业务项目里手动再写一遍。

如果 `mymuduo` 安装在非系统默认前缀下，可以这样告诉 CMake 去哪里找：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/your/install/prefix
```

## 运行示例

项目内置了一个最小可运行的 Echo 服务器示例：`examples/echo_server.cpp`。

```bash
cd build
./echo_server 8888
```

另开一个终端测试：

```bash
nc 127.0.0.1 8888
```

输入任意文本后，服务端会原样回显。

## 说明与后续计划

这是一个学习型实现，重点在于理解 Muduo 的设计思想，而不是完整工业级复刻。后续可以继续补充：

- 示例 Echo/Chat 服务器
- 单元测试与压力测试
- 定时器（TimerQueue）与跨线程任务投递细节优化
- 更完善的错误处理与日志等级

## 致谢

- 陈硕《Linux 多线程服务端编程：使用 muduo C++ 网络库》
- [Muduo 网络库](https://github.com/chenshuo/muduo)