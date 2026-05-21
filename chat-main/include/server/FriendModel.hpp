#pragma once

#include <vector>

#include "User.hpp"

class FriendModel
{
public:
    void insert(int userid, int friendid);
    std::vector<User> query(int userid);
};
