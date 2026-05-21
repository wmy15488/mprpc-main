#include "EventLoopThreadPool.h"

#include "EventLoop.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,
                                         const std::string& nameArg)
    : baseLoop_(baseLoop),
      name_(nameArg),
      started_(false),
      numThreads_(0),
      next_(0),
      threads_(),
      loops_() {}

EventLoopThreadPool::~EventLoopThreadPool()
 {

 }

void EventLoopThreadPool::start(const ThreadInitCallback& cb) 
{
  started_=true;
  for(int i=0;i<numThreads_;++i)
  {
    std::string threadName = name_ + std::to_string(i);
    EventLoopThread*t=new EventLoopThread(cb,threadName);
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->startLoop());
  }
  if(cb&&numThreads_==0)
  {
    cb(baseLoop_);
  }
}

EventLoop* EventLoopThreadPool::getNextLoop() 
{
  EventLoop*next=baseLoop_;
  if(!loops_.empty())
  {
    next=loops_[next_++];
    if(next_>=loops_.size())
    {
      next_=0;
    }
    
  }
  return next;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
  if(numThreads_==0)
  {
    return std::vector<EventLoop*>(1,baseLoop_);
  }
  else
  {
    return loops_;
  }

}
