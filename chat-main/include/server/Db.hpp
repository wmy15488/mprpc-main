#pragma once

#include <mysql/mysql.h>
#include <string>

class Db
{
public:
    Db();
    ~Db();

    bool connect();
    bool update(const std::string& sql);
    MYSQL_RES* query(const std::string& sql);
    MYSQL* getConnection();

private:
    MYSQL* _conn;
};
