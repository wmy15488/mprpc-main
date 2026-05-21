#pragma once
#include<functional>
#include<memory>
#include"nocopyable.h"
class EventLoop;
class Timestamp;

class Channel:nocopyable
{
 public:
 using EventCallback=std::function<void()>;
 using ReadEventCallback=std::function<void(Timestamp)>;
 Channel(EventLoop *loop,int fd);
 ~Channel();
 void handleEvent(Timestamp t);

 void SetreadEventback(ReadEventCallback cb){readEventback_=cb;}
 void SetcloseEventback(EventCallback cb){closeEventback_=cb;}
 void SeterrorEventback(EventCallback cb){errorEventback_=cb;}
 void SetwriteEventback(EventCallback cb){writeEventback_=cb;}

 int index(){return index_;}
 int fd ()const{return fd_;}
 int events()const{return events_;}
 void Setrevents(int revents){revents_=revents;}
 void tie(const std::shared_ptr<void>&obj);

 void ableread(){events_|=kreadEvnet;update();}
 void disableread(){events_&=~kreadEvnet;update();}
 void ablewrite(){events_|=kwriteEvent;update();}
 void disablewrite(){events_&=~kwriteEvent;update();}
 void disableAll(){events_=knoEvent;update();}

 void update();
 void remove();
 void handleEventGuard(Timestamp t);

 void Setindex(int index){index_=index;}
 EventLoop* EventOner(){return loop_;}

 bool iswrite(){return kwriteEvent&events_;}
 bool isread(){return kreadEvnet&events_;}
 bool isnoall(){return knoEvent==events_;}

 private:
 static const int kreadEvnet;
 static const int knoEvent;
 static const int kwriteEvent;
 const int fd_;
 EventLoop*loop_;
 int events_;
 int revents_;
 int index_;
 std::weak_ptr<void> tie_;
 bool tied_;
 ReadEventCallback readEventback_;
 EventCallback closeEventback_;
 EventCallback errorEventback_;
 EventCallback writeEventback_;
};