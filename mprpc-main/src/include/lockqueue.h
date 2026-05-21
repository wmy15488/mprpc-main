#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class LockQueue
{
public:
  LockQueue() : is_close_(false)
  {
  }

  void Push(const T &data)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    if (is_close_)
    {
      return;
    }
    queue_.push(data);
    condition_.notify_one();
  }

  T Pop()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this]()
                    { return !queue_.empty() || is_close_; });
    if (queue_.empty())
    {
      return T();
    }
    T data = queue_.front();
    queue_.pop();
    return data;
  }

  void Close()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    is_close_ = true;
    condition_.notify_all();
  }

private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable condition_;
  bool is_close_;
};
