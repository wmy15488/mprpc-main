#pragma once

#include "nocopyable.h"

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <sys/types.h>

class Thread : public nocopyable {
 public:
  using ThreadFunc = std::function<void()>;

  explicit Thread(ThreadFunc func, const std::string& name = std::string());
  ~Thread();

  void start();
  void join();

  bool started() const { return started_; }
  bool joined() const { return joined_; }
  pid_t tid() const { return tid_; }
  const std::string& name() const { return name_; }

  static int numCreated();

 private:
  void setDefaultName();

  bool started_;
  bool joined_;
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  std::thread thread_;

  static std::atomic_int numCreated_;
};
