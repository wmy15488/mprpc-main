#pragma once
#include"nocopyable.h"
#include<vector>
#include<unordered_map>
#include"Channel.h"

class Poller:nocopyable
{
    public:
    using ChannelList=std::vector<Channel*>;
   
    Poller(EventLoop*loop);
    virtual ~Poller()=default;
    virtual Timestamp poll(int time,ChannelList*activeChannels)=0;
    virtual void updateChannel(Channel*channel)=0;
    virtual void removeChannel(Channel*channel)=0;
    bool hasChannel(Channel*channel)const;
    static Poller*newDefaultPoller(EventLoop*loop);
    protected:
    using ChannelMap=std::unordered_map<int,Channel*>;
    ChannelMap channels_;
    private:
    EventLoop*ownerLoop_;
};