#include "Thread.h"
#include "CurrentThread.h"
#include "Logger.h"
#include <semaphore.h>
#include <cstdio>
#include <utility>

std::atomic_int Thread::numCreated_{0};

Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false),
      joined_(false),
      tid_(0),
      func_(std::move(func)),
      name_(name),
      thread_() 
      {
        setDefaultName();
      }

Thread::~Thread()
 {
  if (started_ && !joined_ && thread_.joinable())
  {
     thread_.detach();
  }
 }

void Thread::start() 
{
 if (started_) {
   LOG_ERORR("Thread::start called twice, name=%s", name_.c_str());
   return;
 }
 started_ = true;

 sem_t sem;
 if (sem_init(&sem, false, 0) != 0) {
   LOG_ERORR("Thread::start sem_init failed, name=%s", name_.c_str());
   started_ = false;
   return;
 }

 try
 {
   ThreadFunc threadFunc = std::move(func_);
   thread_ = std::thread([this, &sem, threadFunc = std::move(threadFunc)]() mutable {
     tid_ = CurrentThread::tid();
     sem_post(&sem);
     threadFunc();
   });
   sem_wait(&sem);
 }
 catch (...)
 {
   started_ = false;
   sem_destroy(&sem);
   throw;
 }

 sem_destroy(&sem);
}

void Thread::join()
{
  if (!started_) {
    LOG_ERORR("Thread::join before start, name=%s", name_.c_str());
    return;
  }
  if (joined_) {
    LOG_ERORR("Thread::join called twice, name=%s", name_.c_str());
    return;
  }
  if (!thread_.joinable()) {
    LOG_ERORR("Thread::join on non-joinable thread, name=%s", name_.c_str());
    return;
  }
  thread_.join();
  joined_ = true;
}

int Thread::numCreated() { return numCreated_; }

void Thread::setDefaultName() 
{
  int num=++numCreated_;
  if(name_.empty())
  {
    char buf[32]={0};
    snprintf(buf,sizeof(buf),"Thread%d",num);
    name_=buf;
  }
}
