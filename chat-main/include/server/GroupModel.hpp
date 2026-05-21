#pragma once

#include <vector>

#include "Group.hpp"

class GroupModel
{
public:
    bool createGroup(Group& group);
    void addGroup(int userid, int groupid, const std::string& role);
    std::vector<Group> queryGroups(int userid);
    std::vector<int> queryGroupUsers(int userid, int groupid);
};
