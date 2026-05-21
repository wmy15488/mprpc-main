#include "ChatService.hpp"
#include "public.hpp"
#include <mymuduo/Logger.h>

#include <string>

using namespace std::placeholders;

void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        _userConnMap.erase(it);
    }

    User user;
    user.setId(userid);
    user.setState("offline");
    _userModel.updateState(user);
    _redis.unsubscribe("chat:user:" + std::to_string(userid));
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    _friendModel.insert(userid, friendid);
    _friendModel.insert(friendid, userid);
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    std::string groupname = js["groupname"];
    std::string groupdesc = js["groupdesc"];

    Group group;
    group.setGroupName(groupname);
    group.setGroupDesc(groupdesc);

    if (_groupModel.createGroup(group))
    {
        _groupModel.addGroup(id, group.getId(), "creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    _groupModel.addGroup(id, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    std::vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        User user=_userModel.query(id);
        if (it != _userConnMap.end()&&user.getState()=="online")
        {
            it->second->send(js.dump());
        }
        else if(user.getState()=="online")
        {
            _redis.publish("chat:user:" + std::to_string(id), js.dump());
        }
        else
        {
            _offlineMsgModel.insert(id, js.dump());
        }
    }
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    std::string pwd = js["password"];
    User user = _userModel.query(id);
    if (user.getId() == 0)
    {
        json response;
        response["msgid"] = LOGIN_MSG;
        response["errno"] = 1;
        response["errmsg"] = "user not exist";
        conn->send(response.dump());
    }
    else if (user.getPwd() != pwd)
    {
        json response;
        response["msgid"] = LOGIN_MSG;
        response["errno"] = 2;
        response["errmsg"] = "password is error";
        conn->send(response.dump());
    }
    else if (user.getState() == "online")
    {
        json response;
        response["msgid"] = LOGIN_MSG;
        response["errno"] = 3;
        response["errmsg"] = "this account is using";
        conn->send(response.dump());
    }
    else
    {
        user.setState("online");
        _userModel.updateState(user);
        _userConnMap.insert({id, conn});
        _redis.subscribe("chat:user:" + std::to_string(id));
        json response;
        response["msgid"] = LOGIN_MSG;
        response["errno"] = 0;
        response["id"] = user.getId();
        response["name"] = user.getName();

        std::vector<std::string> vec = _offlineMsgModel.query(id);
        if (!vec.empty())
        {
            response["offlinemsg"] = vec;
            _offlineMsgModel.remove(id);
        }

        std::vector<User> friendVec = _friendModel.query(id);
        if (!friendVec.empty())
        {
            std::vector<std::string> vec;
            for (User &user : friendVec)
            {
                json js;
                js["id"] = user.getId();
                js["name"] = user.getName();
                js["state"] = user.getState();
                vec.push_back(js.dump());
            }
            response["friends"] = vec;
        }
        std::vector<Group> groupVec = _groupModel.queryGroups(id);
        if (!groupVec.empty())
        {
            std::vector<std::string> groupV;
            for (Group &group : groupVec)
            {
                json grpjs;
                grpjs["id"] = group.getId();
                grpjs["groupname"] = group.getGroupName();
                grpjs["groupdesc"] = group.getGroupDesc();

                std::vector<std::string> userV;
                for (GroupUser &user : group.getUsers())
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    js["role"] = user.getRole();
                    userV.push_back(js.dump());
                }

                grpjs["users"] = userV;
                groupV.push_back(grpjs.dump());
            }

            response["groups"] = groupV;
        }

        conn->send(response.dump());
    }
}

void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string name = js["name"];
    std::string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    user.setState("offline");

    if (_userModel.insert(user))
    {
        json response;
        response["msgid"] = REG_MSG;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = REG_MSG;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

ChatService *ChatService::instance()
{
    static ChatService _service;
    return &_service;
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    int id = -1;
    for (auto it : _userConnMap)
    {
        if (it.second == conn)
        {
            id = it.first;
            break;
        }
    }
    if (id != -1)
    {
        _userConnMap.erase(id);
        User user = _userModel.query(id);
        if (user.getId() != 0)
        {
            user.setState("offline");
            _userModel.updateState(user);
            _redis.unsubscribe("chat:user:" + std::to_string(id));
        }
    }
}

// 单人聊天
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    auto it = _userConnMap.find(toid);
    User user=_userModel.query(toid);

    if (it != _userConnMap.end()&&user.getState()=="online")
    {
        it->second->send(js.dump());
    }
    
    else if(user.getState()=="online")
    {
        _redis.publish("chat:user:" + std::to_string(toid), js.dump());
    }
    else
    {
        _offlineMsgModel.insert(toid,js.dump());
    }
    
       
}

void ChatService::handleRedisSubscribeMessage(const std::string &channel, const std::string &msg)
{
    const std::string prefix = "chat:user:";
    if (channel.rfind(prefix, 0) != 0)
    {
        return;
    }

    int userid = std::stoi(channel.substr(prefix.size()));
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
    }
    else
    {
        _offlineMsgModel.insert(userid, msg);
    }
}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {

        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERORR("not this handler");
        };
    }
    else
        return _msgHandlerMap[msgid];
}
ChatService::ChatService()
{
    if (_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
}
