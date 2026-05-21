# Chat

基于 C++ 实现的聊天服务器项目，支持单机聊天与 `Redis + Nginx` 集群聊天。

## 功能

- 用户注册、登录、退出
- 单聊
- 添加好友
- 离线消息
- 创建群、加群、群聊
- 登录后拉取好友列表、群列表、离线消息
- Redis 发布订阅实现跨服务器消息转发
- Nginx `stream` 实现 TCP 接入层负载均衡

## 技术栈

- C++
- CMake
- MySQL
- Redis
- Nginx
- hiredis
- JSON for Modern C++
- mymuduo

## 项目结构

```text
chat/
├── include/
│   ├── client/
│   └── server/
├── src/
│   ├── client/
│   └── server/
├── thirdparty/
│   └── json/
├── CMakeLists.txt
└── README.md
```

核心模块：

- `src/server/ChatServer.cpp`：网络层入口
- `src/server/ChatService.cpp`：业务层核心
- `src/server/UserModel.cpp`：用户数据访问
- `src/server/FriendModel.cpp`：好友关系数据访问
- `src/server/OfflineMsgModel.cpp`：离线消息数据访问
- `src/server/GroupModel.cpp`：群组数据访问
- `src/server/Redis.cpp`：Redis 发布订阅封装
- `src/client/ChatClient.cpp`：命令行客户端

## 分层设计

- 网络层：`ChatServer` + `mymuduo`
- 业务层：`ChatService`
- 数据层：`Db`、各类 `Model`、`Redis`
- 实体层：`User`、`Group`、`GroupUser`

## 集群实现思路

集群主要分为两部分：

1. Nginx 接入层负载均衡  
客户端统一连接 Nginx 的 `stream` 入口端口，例如 `127.0.0.1:9000`，再由 Nginx 分发到不同 `ChatServer` 实例。

2. Redis 跨服务器消息转发  
用户登录成功后，当前服务器订阅对应的 channel，例如 `chat:user:1`。当另一台服务器发现目标用户在线但不在本机时，会通过 Redis `publish` 转发消息，由真正持有该用户连接的服务器完成最终投递。

## 环境依赖

Ubuntu 下可参考：

```bash
sudo apt update
sudo apt install -y cmake g++ mysql-server redis-server nginx libmysqlclient-dev libhiredis-dev
sudo apt install -y libnginx-mod-stream
```

另外还需要提前安装并配置好 `mymuduo`。

## 编译

```bash
mkdir -p build
cd build
cmake ..
make -j
```

编译完成后会生成：

- `bin/ChatServer`
- `bin/ChatClient`

## 数据库准备

请先创建聊天数据库，并准备以下数据表：

- `user`
- `friend`
- `offlinemessage`
- `allgroup`
- `groupuser`

示例：

```sql
create database chat;
use chat;
```

数据库连接配置位于：

- `src/server/Db.cpp`

请按本地环境修改用户名、密码和字符集。

## 单机启动

1. 启动 MySQL

```bash
sudo systemctl start mysql
```

2. 启动服务端

```bash
cd /home/tian/code/chat
./bin/ChatServer 8000
```

3. 启动客户端

```bash
./bin/ChatClient
./bin/ChatClient 8000
./bin/ChatClient 127.0.0.1 8000
```

默认连接：

```text
127.0.0.1:8000
```

## 集群启动

1. 启动 MySQL

```bash
sudo systemctl start mysql
```

2. 启动 Redis

```bash
sudo systemctl start redis-server
redis-cli ping
```

返回 `PONG` 说明正常。

3. 配置并启动 Nginx

示例 `stream` 配置：

```nginx
stream {
    upstream chatserver {
        server 127.0.0.1:8000;
        server 127.0.0.1:8001;
    }

    server {
        listen 9000;
        proxy_pass chatserver;
        proxy_connect_timeout 10s;
        proxy_timeout 300s;
    }
}
```

检查并重启：

```bash
sudo nginx -t
sudo systemctl restart nginx
```

4. 启动两个服务端实例

```bash
cd /home/tian/code/chat
./bin/ChatServer 8000
./bin/ChatServer 8001
```

5. 客户端通过 Nginx 入口连接

```bash
./bin/ChatClient 9000
./bin/ChatClient 127.0.0.1 9000
```

## 测试建议

单机版：

1. 注册
2. 登录
3. 添加好友
4. 单聊
5. 离线消息
6. 创建群
7. 加群
8. 群聊

集群版：

1. 启动两个 `ChatServer`
2. 客户端连接 `9000`
3. 确认连接被分发到不同服务器
4. 测试跨服单聊
5. 测试跨服群聊
6. 测试跨服离线消息

## 项目亮点

- 支持单机与集群两种运行模式
- 支持好友、群组、离线消息等完整聊天核心能力
- 使用 Redis 发布订阅解决跨服务器消息转发
- 使用 Nginx `stream` 实现 TCP 负载均衡
- 客户端支持命令行参数指定 IP 和端口，方便测试

## 后续可优化方向

- 完善日志系统
- 优化异常处理与容错能力
- 改进客户端交互体验
- 增加更完整的好友 / 群管理功能
- 优化协议设计，处理粘包 / 半包
- 补充自动化测试
