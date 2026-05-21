#include "Poller.h"

#include "EPollPoller.h"
#include "Logger.h"

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
#ifdef MUDUO_USE_POLL
    LOG_FATAL("MUDUO_USE_POLL is defined, but PollPoller is not implemented");
    return nullptr;//生成poll的实例
#else
    return new EPollPoller(loop);//生成epoll的实例
#endif
}
