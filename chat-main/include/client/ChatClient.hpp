#pragma once

#include "json.hpp"

#include <string>
#include <thread>
#include <unordered_map>

using json = nlohmann::json;

class ChatClient
{
public:
    ChatClient();

    bool connectServer(const std::string& ip, unsigned short port);
    void run(const std::string& ip = "127.0.0.1", unsigned short port = 8000);

private:
    void mainMenu();
    void login();
    void reg();
    void loginout();
    void oneChat();
    void addFriend();
    void createGroup();
    void addGroup();
    void groupChat();
    void showCurrentUserData(const json&);
    void help();
    void readTaskHandler();

private:
    int _clientfd;
    bool _isLoginSuccess;
    int _currentUserId;
    std::string _currentUserName;
    std::string _serverIp;
    unsigned short _serverPort;
    std::thread _readTask;
};
