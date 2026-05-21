#include "Db.hpp"

#include <mymuduo/Logger.h>

Db::Db()
{
    _conn = mysql_init(nullptr);
}

Db::~Db()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}

bool Db::connect()
{
    MYSQL* p = mysql_real_connect(_conn,
                                  "127.0.0.1",
                                  "root",
                                  "123456",
                                  "chat",
                                  3306,
                                  nullptr,
                                  0);
    if (p != nullptr)
    {
        mysql_query(_conn, "set names gbk");
        LOG_INFO("connect mysql success!");
        return true;
    }

    LOG_ERORR("connect mysql fail!");
    return false;
}

bool Db::update(const std::string& sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_ERORR("mysql update fail! sql:%s", sql.c_str());
        return false;
    }

    return true;
}

MYSQL_RES* Db::query(const std::string& sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_ERORR("mysql query fail! sql:%s", sql.c_str());
        return nullptr;
    }

    return mysql_use_result(_conn);
}

MYSQL* Db::getConnection()
{
    return _conn;
}
