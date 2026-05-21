#pragma once
#include <functional>
#include <json.hpp>
#include <unordered_map>
#include <mymuduo/TcpConnection.h>
#include <mymuduo/Timestamp.h>
#include "GroupModel.hpp"
#include "UserModel.hpp"
#include "OfflineMsgModel.hpp"
#include "FriendModel.hpp"
#include "Redis.hpp"
using json = nlohmann::json;
using TcpConnectionPtr = TcpConnection::TcpConnectionPtr;
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;
class ChatService
{
public:
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    static ChatService *instance();
    void clientCloseException(const TcpConnectionPtr &conn);
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void handleRedisSubscribeMessage(const std::string& channel, const std::string& msg);

    MsgHandler getHandler(int);

private:
    ChatService();
    std::unordered_map<int, MsgHandler> _msgHandlerMap;
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
    Redis _redis;
    std::unordered_map<int, TcpConnectionPtr> _userConnMap;
};
