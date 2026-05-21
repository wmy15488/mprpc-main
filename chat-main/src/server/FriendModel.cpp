#include "FriendModel.hpp"
#include "Db.hpp"

void FriendModel::insert(int userid, int friendid)
{
    Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql,
                "insert into friend values(%d, %d)",
                userid, friendid);
        db.update(sql);
    }
}

std::vector<User> FriendModel::query(int userid)
{
    std::vector<User> vec;
    Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql,
                "select a.id, a.name, a.state "
                "from user a inner join friend b on b.friendid = a.id "
                "where b.userid = %d",
                userid);

        MYSQL_RES *res = db.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
