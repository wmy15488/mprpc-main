#include "UserModel.hpp"
#include "Db.hpp"
bool UserModel::insert(User &user)
{
    Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql,
                "insert into user(name,password,state) values('%s','%s','%s')",
                user.getName().c_str(),
                user.getPwd().c_str(),
                user.getState().c_str());
        if (db.update(sql))
        {
            user.setId(mysql_insert_id(db.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql,
                "select * from user where id='%d'",
                id);
        MYSQL_RES *res = db.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
            mysql_free_result(res);
        }
    }
    return User();
}

bool UserModel::updateState(User user)
{
    Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql,
                "update user set state='%s' where id='%d'",
                user.getState().c_str(), user.getId());
                if(db.update(sql))
                {
                    return true;
                }
    }
    return false;
}

void UserModel::resetState()
{
     Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql, "update user set state='offline' where state='online'");
        db.update(sql);
    }
}
