#include "Redis.hpp"

#include <mymuduo/Logger.h>

#include <string>
#include <thread>

Redis::Redis()
    : _publish_context(nullptr), _subscribe_context(nullptr)
{
}

Redis::~Redis()
{
    if (_publish_context != nullptr)
    {
        redisFree(_publish_context);
        _publish_context = nullptr;
    }

    if (_subscribe_context != nullptr)
    {
        redisFree(_subscribe_context);
        _subscribe_context = nullptr;
    }
}

bool Redis::connect()
{
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (_publish_context == nullptr || _publish_context->err)
    {
        if (_publish_context != nullptr)
        {
            LOG_ERORR("redis publish connect error:%s", _publish_context->errstr);
            redisFree(_publish_context);
            _publish_context = nullptr;
        }
        return false;
    }

    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if (_subscribe_context == nullptr || _subscribe_context->err)
    {
        if (_subscribe_context != nullptr)
        {
            LOG_ERORR("redis subscribe connect error:%s", _subscribe_context->errstr);
            redisFree(_subscribe_context);
            _subscribe_context = nullptr;
        }

        redisFree(_publish_context);
        _publish_context = nullptr;
        return false;
    }

    std::thread observer_thread([this]() { observer_channel_message(); });
    observer_thread.detach();

    LOG_INFO("connect redis-server success!");
    return true;
}

bool Redis::publish(const std::string& channel, const std::string& message)
{
    if (_publish_context == nullptr)
    {
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(
        redisCommand(_publish_context, "PUBLISH %s %s", channel.c_str(), message.c_str()));
    if (reply == nullptr)
    {
        LOG_ERORR("redis publish error, channel:%s", channel.c_str());
        return false;
    }

    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(const std::string& channel)
{
    if (_subscribe_context == nullptr)
    {
        return false;
    }

    if (REDIS_ERR == redisAppendCommand(_subscribe_context, "SUBSCRIBE %s", channel.c_str()))
    {
        LOG_ERORR("redis subscribe append command error, channel:%s", channel.c_str());
        return false;
    }

    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(_subscribe_context, &done))
        {
            LOG_ERORR("redis subscribe buffer write error, channel:%s", channel.c_str());
            return false;
        }
    }

    return true;
}

bool Redis::unsubscribe(const std::string& channel)
{
    if (_subscribe_context == nullptr)
    {
        return false;
    }

    if (REDIS_ERR == redisAppendCommand(_subscribe_context, "UNSUBSCRIBE %s", channel.c_str()))
    {
        LOG_ERORR("redis unsubscribe append command error, channel:%s", channel.c_str());
        return false;
    }

    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(_subscribe_context, &done))
        {
            LOG_ERORR("redis unsubscribe buffer write error, channel:%s", channel.c_str());
            return false;
        }
    }

    return true;
}

void Redis::init_notify_handler(std::function<void(const std::string&, const std::string&)> fn)
{
    _notify_message_handler = std::move(fn);
}

void Redis::observer_channel_message()
{
    redisReply* reply = nullptr;
    while (REDIS_OK == redisGetReply(_subscribe_context, reinterpret_cast<void**>(&reply)))
    {
        if (reply != nullptr &&
            reply->type == REDIS_REPLY_ARRAY &&
            reply->elements == 3 &&
            reply->element[0] != nullptr &&
            reply->element[1] != nullptr &&
            reply->element[2] != nullptr &&
            reply->element[0]->str != nullptr &&
            std::string(reply->element[0]->str) == "message")
        {
            std::string channel = reply->element[1]->str;
            std::string message = reply->element[2]->str;

            if (_notify_message_handler)
            {
                _notify_message_handler(channel, message);
            }
        }

        if (reply != nullptr)
        {
            freeReplyObject(reply);
            reply = nullptr;
        }
    }
}
