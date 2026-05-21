#pragma once

#include <functional>
#include <string>

#include <hiredis/hiredis.h>

class Redis
{
public:
    Redis();
    ~Redis();

    bool connect();
    bool publish(const std::string& channel, const std::string& message);
    bool subscribe(const std::string& channel);
    bool unsubscribe(const std::string& channel);

    void init_notify_handler(std::function<void(const std::string&, const std::string&)> fn);

private:
    void observer_channel_message();

private:
    redisContext* _publish_context;
    redisContext* _subscribe_context;
    std::function<void(const std::string&, const std::string&)> _notify_message_handler;
};
