#include "User.hpp"

User::User()
    : _id(0), _state("offline")
{
}

User::User(int id, const std::string& name, const std::string& pwd, const std::string& state)
    : _id(id), _name(name), _pwd(pwd), _state(state)
{
}

void User::setId(int id)
{
    _id = id;
}

void User::setName(const std::string& name)
{
    _name = name;
}

void User::setPwd(const std::string& pwd)
{
    _pwd = pwd;
}

void User::setState(const std::string& state)
{
    _state = state;
}

int User::getId() const
{
    return _id;
}

std::string User::getName() const
{
    return _name;
}

std::string User::getPwd() const
{
    return _pwd;
}

std::string User::getState() const
{
    return _state;
}
