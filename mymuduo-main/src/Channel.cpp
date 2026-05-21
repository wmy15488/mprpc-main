#include "Channel.h"
#include "EventLoop.h"
#include "Timestamp.h"
#include <sys/epoll.h>

const int Channel::knoEvent=0;
const int Channel::kreadEvnet=EPOLLIN|EPOLLPRI;
const int Channel::kwriteEvent=EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
:loop_(loop),fd_(fd),index_(-1),events_(0),revents_(0),tied_(false)
{
}

Channel::~Channel()
{
}

void Channel::handleEvent(Timestamp t)
{
    // 如果设置了 tie_，先尝试提升 shared_ptr，避免对象析构后仍被回调访问。
    if(tied_)
    {
        std::shared_ptr<void>guard=tie_.lock();
        if(guard)
        {
            handleEventGuard(t);
        }
    }
    else
    {
        handleEventGuard(t);
    }
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    // 绑定生命周期托管对象，防止回调执行期间对象被销毁。
    tie_=obj;
    tied_=true;
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::remove()
{
    loop_->removeChannel(this);
}

void Channel::handleEventGuard(Timestamp t)
{
    // 对端关闭（且当前没有可读数据）时触发关闭回调。
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (closeEventback_) closeEventback_();
    }

    // epoll 报错事件。
    if (revents_ & EPOLLERR) {
        if (errorEventback_) errorEventback_();
    }

    // 可读事件：普通读、带外数据、对端半关闭。
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readEventback_) readEventback_(t);
    }

    // 可写事件。
    if (revents_ & EPOLLOUT) {
        if (writeEventback_) writeEventback_();
    }
}
