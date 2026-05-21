#pragma once

#include <string>
#include <vector>

class GroupUser
{
public:
    GroupUser();

    void setId(int id);
    void setName(const std::string& name);
    void setState(const std::string& state);
    void setRole(const std::string& role);

    int getId() const;
    std::string getName() const;
    std::string getState() const;
    std::string getRole() const;

private:
    int _id;
    std::string _name;
    std::string _state;
    std::string _role;
};

class Group
{
public:
    Group();
    Group(int id, const std::string& groupname, const std::string& groupdesc);

    void setId(int id);
    void setGroupName(const std::string& groupname);
    void setGroupDesc(const std::string& groupdesc);

    int getId() const;
    std::string getGroupName() const;
    std::string getGroupDesc() const;

    std::vector<GroupUser>& getUsers();

private:
    int _id;
    std::string _groupname;
    std::string _groupdesc;
    std::vector<GroupUser> _users;
};
