#pragma once

#include "User.hpp"

class UserModel
{
public:
    bool insert(User& user);
    User query(int id);
    bool updateState(User user);
    void resetState();
};
