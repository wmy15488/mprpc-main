#pragma once

#include "nocopyable.h"
#include <algorithm>
#include <cassert>
#include <string>
#include <sys/types.h>
#include <vector>

class Buffer : nocopyable
{
 public:
  // 默认构造一个带预留区的缓冲区。
  explicit Buffer(size_t initialSize = kInitialSize);

  // 返回可读数据的起始地址。
  const char* peek() const;

  // 返回可读数据的长度。
  size_t readableBytes() const;

  // 返回可写空间的长度。
  size_t writableBytes() const;

  // 返回头部预留区的长度。
  size_t prependableBytes() const;

  // 按长度回收已读数据。
  void retrieve(size_t len);

  // 回收全部已读数据。
  void retrieveAll();

  // 以字符串形式取出全部可读数据。
  std::string retrieveAllAsString();

  // 追加字符串数据。
  void append(const std::string& str);

  // 追加指定长度的数据。
  void append(const char* data, size_t len);

  // 从文件描述符读取数据到缓冲区。
  ssize_t readFd(int fd, int* saveErrno);

  // 确保缓冲区存在足够可写空间。
  void ensureWritableBytes(size_t len);

  // 返回可写区域的起始地址。
  char* beginWrite();

  // 返回可写区域的起始地址。
  const char* beginWrite() const;

  // 标记已经写入的数据长度。
  void hasWritten(size_t len);

 private:
  // 返回底层容器的起始地址。
  char* begin();

  // 返回底层容器的起始地址。
  const char* begin() const;

  // 扩容或整理缓冲区空间。
  void makeSpace(size_t len);

  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;
};
