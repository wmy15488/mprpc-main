#include "OfflineMsgModel.hpp"
#include "Db.hpp"
#include <vector>
void OfflineMsgModel::insert(int userid, const std::string &msg)
{
    Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql,
                "insert into offlinemessage values('%d','%s')",
                userid, msg.c_str());
        db.update(sql);
    }
}

void OfflineMsgModel::remove(int userid)
{
    Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql, "delete from offlinemessage where userid=%d", userid);
        db.update(sql);
    }
}

std::vector<std::string> OfflineMsgModel::query(int userid)
{
    std::vector<std::string> vec;
    Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql,
                "select message from offlinemessage where userid=%d",
                userid);
                MYSQL_RES* res=db.query(sql);
                if(res!=nullptr)
                {
                    MYSQL_ROW row;
                    while((row=mysql_fetch_row(res))!=nullptr)
                    {
                        vec.push_back(row[0]);
                    }
                    mysql_free_result(res);
                }
    }
    return vec;
}
