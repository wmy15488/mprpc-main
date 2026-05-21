#include "ChatClient.hpp"
#include "public.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <string>
#include <strings.h>

ChatClient::ChatClient()
    : _clientfd(-1),
      _isLoginSuccess(false),
      _currentUserId(0),
      _serverIp("127.0.0.1"),
      _serverPort(8000)
{
}

bool ChatClient::connectServer(const std::string &ip, unsigned short port)
{
    _clientfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (_clientfd == -1)
    {
        std::cerr << "create socket error" << std::endl;
        return false;
    }

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (::connect(_clientfd, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "connect server error" << std::endl;
        ::close(_clientfd);
        _clientfd = -1;
        return false;
    }
    return true;
}

void ChatClient::run(const std::string& ip, unsigned short port)
{
    _serverIp = ip;
    _serverPort = port;

    if (!connectServer(_serverIp, _serverPort))
    {
        return;
    }

    mainMenu();
}

void ChatClient::mainMenu()
{
    while (true)
    {
        std::cout << "====================" << std::endl;
        std::cout << "1. login" << std::endl;
        std::cout << "2. reg" << std::endl;
        std::cout << "3. quit" << std::endl;
        std::cout << "====================" << std::endl;
        std::cout << "choice: ";

        int choice = 0;
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            login();
            break;
        case 2:
            reg();
            break;
        case 3:
            _isLoginSuccess = false;
            if (_clientfd != -1)
            {
                ::shutdown(_clientfd, SHUT_RDWR);
                ::close(_clientfd);
                _clientfd = -1;
            }
            if (_readTask.joinable())
            {
                _readTask.join();
            }
            return;
        default:
            std::cout << "invalid input!" << std::endl;
            break;
        }
    }
}

void ChatClient::login()
{
    if (_clientfd == -1 && !connectServer(_serverIp, _serverPort))
    {
        return;
    }

    int id = 0;
    std::string password;

    std::cout << "userid: ";
    std::cin >> id;
    std::cout << "password: ";
    std::cin >> password;

    json js;
    js["msgid"] = LOGIN_MSG;
    js["id"] = id;
    js["password"] = password;

    std::string request = js.dump();
    int len = ::write(_clientfd, request.data(), request.size());
    if (len == -1)
    {
        std::cerr << "send login msg error" << std::endl;
        return;
    }

    char buffer[4096] = {0};
    len = ::read(_clientfd, buffer, sizeof(buffer));
    if (len == -1)
    {
        std::cerr << "read login response error" << std::endl;
        return;
    }

    json response = json::parse(buffer);
    if (response["errno"].get<int>() != 0)
    {
        std::cout << response["errmsg"] << std::endl;
        return;
    }

    _isLoginSuccess = true;
    _currentUserId = response["id"].get<int>();
    _currentUserName = response["name"];
    std::cout << "login success!" << std::endl;
    showCurrentUserData(response);
    help();
    if (_readTask.joinable())
    {
        _readTask.join();
    }
    _readTask = std::thread(&ChatClient::readTaskHandler, this);
    std::cin.get();
    while (_isLoginSuccess)
    {
        std::cout << "command > ";
        std::string command;
        std::getline(std::cin, command);

        if (command == "help")
        {
            help();
        }
        else if (command == "chat")
        {
            oneChat();
        }
        else if (command == "addfriend")
        {
            addFriend();
        }
        else if (command == "creategroup")
        {
            createGroup();
        }
        else if (command == "addgroup")
        {
            addGroup();
        }
        else if (command == "groupchat")
        {
            groupChat();
        }
        else if (command == "loginout")
        {
            loginout();
        }
        else if (!command.empty())
        {
            std::cout << "invalid command!" << std::endl;
        }
    }
}

void ChatClient::reg()
{
    if (_clientfd == -1 && !connectServer(_serverIp, _serverPort))
    {
        return;
    }

    std::string name;
    std::string password;

    std::cout << "username: ";
    std::cin >> name;
    std::cout << "password: ";
    std::cin >> password;

    json js;
    js["msgid"] = REG_MSG;
    js["name"] = name;
    js["password"] = password;

    std::string request = js.dump();
    int len = ::send(_clientfd, request.data(), request.size(), 0);

    if (len == -1)
    {
        std::cerr << "send reg response error" << std::endl;
        return;
    }
    char buffer[1024] = {0};
    len = ::recv(_clientfd, buffer, 1024, 0);
    if (len == -1)
    {
        std::cerr << "recv reg response error" << std::endl;
        return;
    }

    json response = json::parse(buffer);
    if (response["errno"].get<int>() == 0)
    {
        std::cout << "reg success, userid is " << response["id"] << std::endl;
    }
    else
    {
        std::cout << "reg failed" << std::endl;
    }
}

void ChatClient::loginout()
{
    if (!_isLoginSuccess)
    {
        return;
    }

    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = _currentUserId;

    std::string request = js.dump();
    int len = ::write(_clientfd, request.data(), request.size());
    if (len == -1)
    {
        std::cerr << "send loginout msg error" << std::endl;
        return;
    }

    _isLoginSuccess = false;
    _currentUserId = 0;
    _currentUserName.clear();
    if (_clientfd != -1)
    {
        ::shutdown(_clientfd, SHUT_RDWR);
        ::close(_clientfd);
        _clientfd = -1;
    }
    if (_readTask.joinable())
    {
        _readTask.join();
    }

    std::cout << "logout success!" << std::endl;
}

void ChatClient::oneChat()
{
    int toid = 0;
    std::cout << "friendid: ";
    std::cin >> toid;
    std::cin.get();

    std::string message;
    std::cout << "message: ";
    std::getline(std::cin, message);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = _currentUserId;
    js["toid"] = toid;
    js["msg"] = message;

    std::string request = js.dump();
    int len = ::write(_clientfd, request.data(), request.size());
    if (len == -1)
    {
        std::cerr << "send one chat msg error" << std::endl;
    }
}

void ChatClient::addFriend()
{
    int friendid = 0;
    std::cout << "friendid: ";
    std::cin >> friendid;
    std::cin.get();

    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = _currentUserId;
    js["friendid"] = friendid;

    std::string request = js.dump();
    int len = ::write(_clientfd, request.data(), request.size());
    if (len == -1)
    {
        std::cerr << "send add friend msg error" << std::endl;
        return;
    }

    std::cout << "add friend request sent." << std::endl;
}

void ChatClient::createGroup()
{
    std::string groupname;
    std::string groupdesc;

    std::cout << "groupname: ";
    std::getline(std::cin, groupname);
    std::cout << "groupdesc: ";
    std::getline(std::cin, groupdesc);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = _currentUserId;
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;

    std::string request = js.dump();
    int len = ::write(_clientfd, request.data(), request.size());
    if (len == -1)
    {
        std::cerr << "send create group msg error" << std::endl;
        return;
    }

    std::cout << "create group request sent." << std::endl;
}

void ChatClient::addGroup()
{
    int groupid = 0;
    std::cout << "groupid: ";
    std::cin >> groupid;
    std::cin.get();

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = _currentUserId;
    js["groupid"] = groupid;

    std::string request = js.dump();
    int len = ::write(_clientfd, request.data(), request.size());
    if (len == -1)
    {
        std::cerr << "send add group msg error" << std::endl;
        return;
    }

    std::cout << "add group request sent." << std::endl;
}

void ChatClient::groupChat()
{
    int groupid = 0;
    std::cout << "groupid: ";
    std::cin >> groupid;
    std::cin.get();

    std::string message;
    std::cout << "message: ";
    std::getline(std::cin, message);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = _currentUserId;
    js["groupid"] = groupid;
    js["msg"] = message;

    std::string request = js.dump();
    int len = ::write(_clientfd, request.data(), request.size());
    if (len == -1)
    {
        std::cerr << "send group chat msg error" << std::endl;
    }
}

void ChatClient::showCurrentUserData(const json &response)
{
    std::cout << "================ current user ================" << std::endl;
    std::cout << "id: " << response["id"] << std::endl;
    std::cout << "name: " << response["name"] << std::endl;

    if (response.contains("offlinemsg"))
    {
        std::cout << "================ offline messages ================" << std::endl;
        for (std::string msg : response["offlinemsg"])
        {
            std::cout << msg << std::endl;
        }
    }

    if (response.contains("friends"))
    {
        std::cout << "================ friends ================" << std::endl;
        for (std::string str : response["friends"])
        {
            json js = json::parse(str);
            std::cout << "id:" << js["id"]
                      << " name:" << js["name"]
                      << " state:" << js["state"] << std::endl;
        }
    }

    if (response.contains("groups"))
    {
        std::cout << "================ groups ================" << std::endl;
        for (std::string str : response["groups"])
        {
            json grpjs = json::parse(str);
            std::cout << "group id:" << grpjs["id"]
                      << " group name:" << grpjs["groupname"]
                      << " group desc:" << grpjs["groupdesc"] << std::endl;

            if (grpjs.contains("users"))
            {
                for (std::string userstr : grpjs["users"])
                {
                    json userjs = json::parse(userstr);
                    std::cout << "    member id:" << userjs["id"]
                              << " name:" << userjs["name"]
                              << " state:" << userjs["state"]
                              << " role:" << userjs["role"] << std::endl;
                }
            }
        }
    }
}

void ChatClient::help()
{
    std::cout << "show command list:" << std::endl;
    std::cout << "help : show all command" << std::endl;
    std::cout << "chat : one to one chat" << std::endl;
    std::cout << "addfriend : add friend" << std::endl;
    std::cout << "creategroup : create group" << std::endl;
    std::cout << "addgroup : add group" << std::endl;
    std::cout << "groupchat : group chat" << std::endl;
    std::cout << "loginout : logout" << std::endl;
}

void ChatClient::readTaskHandler()
{
    while (_isLoginSuccess)
    {
        char buffer[1024] = {0};
        int len = ::read(_clientfd, buffer, sizeof(buffer));
        if (len <= 0)
        {
            if (_isLoginSuccess)
            {
                std::cerr << "read message error" << std::endl;
            }
            break;
        }
        json js = json::parse(buffer);
        int msgid = js["msgid"].get<int>();
        if (msgid == ONE_CHAT_MSG)
        {
            std::cout << "\n[one chat] user " << js["id"]
                      << " said: " << js["msg"] << std::endl;
        }
        else if (msgid == GROUP_CHAT_MSG)
        {
            std::cout << "\n[group chat] group " << js["groupid"]
                      << " user " << js["id"]
                      << " said: " << js["msg"] << std::endl;
        }
        std::cout<<"command >"<<std::flush;
    }
}
