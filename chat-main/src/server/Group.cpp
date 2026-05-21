#include "Group.hpp"

GroupUser::GroupUser()
    : _id(0)
{
}

void GroupUser::setId(int id)
{
    _id = id;
}

void GroupUser::setName(const std::string &name)
{
    _name = name;
}

void GroupUser::setState(const std::string &state)
{
    _state = state;
}

void GroupUser::setRole(const std::string &role)
{
    _role = role;
}

int GroupUser::getId() const
{
    return _id;
}

std::string GroupUser::getName() const
{
    return _name;
}

std::string GroupUser::getState() const
{
    return _state;
}

std::string GroupUser::getRole() const
{
    return _role;
}

Group::Group()
    : _id(0)
{
}

Group::Group(int id, const std::string &groupname, const std::string &groupdesc)
    : _id(id), _groupname(groupname), _groupdesc(groupdesc)
{
}

void Group::setId(int id)
{
    _id = id;
}

void Group::setGroupName(const std::string &groupname)
{
    _groupname = groupname;
}

void Group::setGroupDesc(const std::string &groupdesc)
{
    _groupdesc = groupdesc;
}

int Group::getId() const
{
    return _id;
}

std::string Group::getGroupName() const
{
    return _groupname;
}

std::string Group::getGroupDesc() const
{
    return _groupdesc;
}

std::vector<GroupUser> &Group::getUsers()
{
    return _users;
}
