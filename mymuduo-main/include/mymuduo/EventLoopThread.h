#pragma once

#include "Thread.h"
#include "nocopyable.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

class EventLoop;

class EventLoopThread : public nocopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  explicit EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                           const std::string& name = std::string());
  ~EventLoopThread();

  EventLoop* startLoop();

 private:
  void threadFunc();

  EventLoop* loop_;
  bool exiting_;
  Thread loopThread_;
  std::mutex mutex_;
  std::condition_variable cond_;
  ThreadInitCallback callback_;
};
