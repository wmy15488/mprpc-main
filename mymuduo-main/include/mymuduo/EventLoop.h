#pragma once
#include "nocopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"
#include "Logger.h"
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <sys/eventfd.h>
#include <unistd.h>
#include <stdint.h>

class Channel;
class Poller;

class EventLoop : public nocopyable {
 public:
  using Function = std::function<void()>;
  using ChannelList = std::vector<Channel*>;

  EventLoop();
  ~EventLoop();

  void loop();
  void quit();

  Timestamp pollReturnTime() const { return pollReturnTime_; }

  void runInLoop(Function cb);
  void queueInLoop(Function cb);

  void wakeup();
  
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);
  bool hasChannel(Channel* channel);

  bool isInLoopThread() const;
  

 private:
  void handleRead();
  void doPendingFunctions();
  int creatweakfd();
  static constexpr int kPollerTimeoutMs = 1000;

  std::atomic_bool looping_;
  std::atomic_bool quit_;
  const int threadId_;

  Timestamp pollReturnTime_;
  std::unique_ptr<Poller> poller_;
  int wakeupFd_;
  std::unique_ptr<Channel> wakeupChannel_;
  ChannelList activeChannels_;
  std::atomic_bool callingPendingFunctions_;
  std::vector<Function> pendingFunctions_;
  std::mutex mutex_;
};
