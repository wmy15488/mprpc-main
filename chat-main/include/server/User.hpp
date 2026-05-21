#pragma once

#include <string>

class User
{
public:
    User();
    User(int id, const std::string& name, const std::string& pwd, const std::string& state);

    void setId(int id);
    void setName(const std::string& name);
    void setPwd(const std::string& pwd);
    void setState(const std::string& state);

    int getId() const;
    std::string getName() const;
    std::string getPwd() const;
    std::string getState() const;

private:
    int _id;
    std::string _name;
    std::string _pwd;
    std::string _state;
};
