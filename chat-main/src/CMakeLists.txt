# 收集当前可执行程序需要编译的源文件
# 后面功能变多时，可以继续把更多 .cpp 文件加到这里
set(CHAT_SERVER_SOURCES
    server/main.cpp
    server/ChatServer.cpp
    server/ChatService.cpp
    server/Redis.cpp
    server/Db.cpp
    server/User.cpp
    server/UserModel.cpp
    server/OfflineMsgModel.cpp
    server/FriendModel.cpp
    server/Group.cpp
    server/GroupModel.cpp
)

set(CHAT_CLIENT_SOURCES
    client/main.cpp
    client/ChatClient.cpp
)

# 根据上面的源文件生成一个可执行程序，程序名叫 ChatServer
add_executable(ChatServer ${CHAT_SERVER_SOURCES})

add_executable(ChatClient ${CHAT_CLIENT_SOURCES})

# 给 ChatServer 这个目标单独指定头文件搜索路径
target_include_directories(ChatServer
    PRIVATE
    # 我们自己写的头文件目录
    ${PROJECT_SOURCE_DIR}/include
    # server 目录下的头文件允许直接写 #include "ChatServer.hpp"
    ${PROJECT_SOURCE_DIR}/include/server
    # 第三方头文件目录，这里放 json.hpp
    ${PROJECT_SOURCE_DIR}/thirdparty
    # json 单头文件所在目录，允许直接写 #include <json.hpp>
    ${PROJECT_SOURCE_DIR}/thirdparty/json
    # mymuduo 头文件目录
    ${MYMUDUO_INCLUDE_DIR}
    # MySQL 头文件目录
    /usr/include/mysql
)

target_include_directories(ChatClient
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/client
    ${PROJECT_SOURCE_DIR}/include/server
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/thirdparty/json
)

# 在指定目录中查找名为 mymuduo 的库
# 找到后会把完整路径保存到 MYMUDUO_LIB 变量中
# REQUIRED 表示找不到就直接报错，避免后面链接阶段才发现问题
find_library(MYMUDUO_LIB
    NAMES mymuduo
    HINTS ${MYMUDUO_LIBRARY_DIR}
    REQUIRED
)

# 查找 MySQL 客户端库，Db.cpp 会通过它调用 mysql_real_connect 等接口
find_library(MYSQLCLIENT_LIB
    NAMES mysqlclient
    HINTS /usr/lib/x86_64-linux-gnu
    REQUIRED
)

find_library(HIREDIS_LIB
    NAMES hiredis
    HINTS /usr/lib/x86_64-linux-gnu /usr/lib
    REQUIRED
)

# 指定 ChatServer 在链接阶段需要依赖哪些库
target_link_libraries(ChatServer
    # 链接 mymuduo 动态库
    ${MYMUDUO_LIB}
    # 链接 MySQL 客户端库
    ${MYSQLCLIENT_LIB}
    # 链接 hiredis 客户端库
    ${HIREDIS_LIB}
    # 链接 pthread，因为网络库和多线程通常会依赖它
    pthread
)

target_link_libraries(ChatClient
    pthread
)
