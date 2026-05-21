# 指定 CMake 的最低版本要求，版本太低会有一些语法和功能不可用
cmake_minimum_required(VERSION 3.16)

# 定义项目名称、版本号，以及这是一个 C++ 项目
project(chatserver VERSION 1.0 LANGUAGES CXX)

# 指定使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
# 要求编译器必须支持上面指定的标准，不能偷偷降级
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 关闭编译器的 GNU 扩展，尽量使用更标准的 C++ 语法
set(CMAKE_CXX_EXTENSIONS OFF)
# 导出 compile_commands.json，方便 clangd、VS Code 等工具做代码补全和跳转
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 指定可执行文件输出到项目根目录下的 bin 文件夹
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 指定 mymuduo 头文件目录，默认放在 /usr/local/include
# CACHE PATH 的意思是把这个变量放进 CMake 缓存，后续可以很方便地修改
set(MYMUDUO_INCLUDE_DIR "/usr/local/include" CACHE PATH "mymuduo include directory")
# 指定 mymuduo 库文件目录，默认放在 /usr/local/lib
set(MYMUDUO_LIBRARY_DIR "/usr/local/lib" CACHE PATH "mymuduo library directory")

# 添加我们自己项目的头文件搜索路径，比如 #include "server/ChatServer.hpp"
include_directories(${PROJECT_SOURCE_DIR}/include)
# 添加第三方头文件搜索路径，这里主要是给 json.hpp 用
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
# 添加 mymuduo 的头文件搜索路径，比如 #include <mymuduo/TcpServer.h>
include_directories(${MYMUDUO_INCLUDE_DIR})
# 添加库文件搜索路径，链接器会去这里找 libmymuduo.so
link_directories(${MYMUDUO_LIBRARY_DIR})

# 进入 src 子目录继续处理 src/CMakeLists.txt
add_subdirectory(src)
