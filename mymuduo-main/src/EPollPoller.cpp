#include "EPollPoller.h"
#include"Logger.h"
#include<unistd.h>
const int kNew=-1;
const int kAdded=1;
const int kDeleted=2;
EPollPoller::EPollPoller(EventLoop *loop):Poller(loop),epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
events_(KInitEventListSize)
{
    if(epollfd_<0)
    {
     LOG_FATAL("epoll_create error:%d\n",errno);
    }
}

EPollPoller::~EPollPoller()
{
    close(epollfd_);
}

Timestamp EPollPoller::poll(int time, ChannelList *activeChannels)
{
        int numEvents=::epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),time);
        int saveErrno=errno;
        Timestamp now(Timestamp::now());
        if(numEvents>0)
        {
            fillActiveChannels(numEvents,activeChannels);
            if(numEvents==events_.size())
            events_.resize(events_.size()*2);

        }
        else if(numEvents==0)
        {
            LOG_DBUG("timeout");
        }
        else         
        {
            //errno
        }
        
        return now;
    
}

void EPollPoller::updateChannel(Channel *channel)
{
    const int index=channel->index();
    if(index==kNew||index==kDeleted)
    {
        if(index==kNew)
        {
            int fd=channel->fd();
            channels_[fd]=channel;
        }
        channel->Setindex(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else
    {
        int fd=channel->fd();
        if(channel->isnoall())
    {
        update(EPOLL_CTL_DEL,channel);
        channel->Setindex(kDeleted);
    }
    else
    {
        update(EPOLL_CTL_MOD,channel);
    }
    }
}

void EPollPoller::removeChannel(Channel *channel)
{
    int fd =channel->fd();
    channels_.erase(fd);
    int index=channel->index();
    if(index==kAdded)
    {
        update(EPOLL_CTL_DEL,channel);
    }
    channel->Setindex(kNew);
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannel) const
{
    for(int i=0;i<numEvents;i++)
    {
        Channel*channel=static_cast<Channel*>(events_[i].data.ptr);
        channel->Setrevents(events_[i].events);
        activeChannel->push_back(channel);
    }
}

void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    memset(&event,0,sizeof(event));
    event.events=channel->events();
    event.data.ptr=channel;
    int fd=channel->fd();
    if(::epoll_ctl(epollfd_,operation,fd,&event)<0)
    {
        if(operation==EPOLL_CTL_DEL)
        {
            LOG_ERORR("epoll_ctl del error:%d\n",errno);
        }
       else LOG_FATAL("epoll_ctl add/mod error:%d\n",errno);
    }
}