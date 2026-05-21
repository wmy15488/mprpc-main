#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include <errno.h>

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(creatweakfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      callingPendingFunctions_(false)
{
  wakeupChannel_->SetreadEventback([this](Timestamp) { handleRead(); });
  wakeupChannel_->ableread();
}

EventLoop::~EventLoop() 
{
    if(wakeupChannel_)
    {
        wakeupChannel_->disableAll();
        wakeupChannel_->remove();
    }
    if(wakeupFd_>=0)
    {
        ::close(wakeupFd_);
        wakeupFd_=-1;
    }
}

void EventLoop::loop() 
{
    looping_=true;
    quit_=false;
    while(!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_=poller_->poll(kPollerTimeoutMs,&activeChannels_);
        for(Channel*channel:activeChannels_)
        {
            channel->handleEvent(pollReturnTime_);
        }
        doPendingFunctions();
    }
    looping_=false;
    
}

void EventLoop::quit()
 {
    quit_=true;
    if(!isInLoopThread())
    {
        wakeup();
    }
 }

void EventLoop::runInLoop(Function cb)
 {
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
 }

void EventLoop::queueInLoop(Function cb)
 {
    {
        std::unique_lock<std::mutex>mtx(mutex_);
        pendingFunctions_.emplace_back(cb);
    }
    if(!isInLoopThread()||callingPendingFunctions_)
    {
        wakeup();
    }
 }

void EventLoop::wakeup()
 {
    uint64_t one=1;
    ssize_t n=::write(wakeupFd_,&one,sizeof(one));
    if(n!=sizeof(one))
    {
        LOG_ERORR("EventLoop::wakeup");
    }
 }

void EventLoop::updateChannel(Channel* channel) 
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
 {
    poller_->removeChannel(channel);
 }

bool EventLoop::hasChannel(Channel* channel)
 {
    return poller_->hasChannel(channel);
 }

bool EventLoop::isInLoopThread() const { return threadId_==CurrentThread::tid(); }



void EventLoop::handleRead() 
{
    uint64_t one =1;
    ssize_t n=::read(wakeupFd_,&one,sizeof(one));
    if(n!=sizeof(one))
    {
        LOG_ERORR("EventLoop::handeRead");
    }
}

void EventLoop::doPendingFunctions()
 {
    std::vector<Function>functions;
    callingPendingFunctions_=true;
    {
        std::unique_lock<std::mutex>mtx(mutex_);
        functions.swap(pendingFunctions_);

    }
    for(const Function&function:functions)
    {
        function();
    }
     callingPendingFunctions_=false;
 }

int EventLoop::creatweakfd()
{
  int wakefd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (wakefd < 0) {
    LOG_FATAL("eventfd create error:%d\n", errno);
  }
  return wakefd;
}
