#pragma once
#include"Poller.h"
#include<sys/epoll.h>
#include"Timestamp.h"
class EPollPoller : public Poller
{
   public:
   EPollPoller(EventLoop*loop);
   ~EPollPoller()override;
   Timestamp poll(int time,ChannelList*activeChannels)override;
   void updateChannel(Channel*channel)override;
   void removeChannel(Channel*channel)override;

   private:
   static const int KInitEventListSize=16;
   void fillActiveChannels(int numEvents,ChannelList*activeChannel)const;
   void update(int operation,Channel*channel);
   using EventList=std::vector<epoll_event>;
   EventList events_;
   int epollfd_;
};
