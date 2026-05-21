#include "GroupModel.hpp"
#include "Db.hpp"
bool GroupModel::createGroup(Group &group)
{
    Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql,
                "insert into allgroup(groupname,groupdesc)"
                "values('%s','%s')",
                group.getGroupName().c_str(), group.getGroupDesc().c_str());
        if (db.update(sql))
        {
            group.setId(mysql_insert_id(db.getConnection()));
            return true;
        }
    }
    return false;
}

void GroupModel::addGroup(int userid, int groupid, const std::string &role)
{
    Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql,
                "insert into groupuser values(%d, %d, '%s')",
                groupid,
                userid,
                role.c_str());

        db.update(sql);
    }
}

std::vector<Group> GroupModel::queryGroups(int userid)
{
    std::vector<Group> groupVec;
    Db db;
    if (db.connect())
    {
        char sql1[1024] = {0};
        sprintf(sql1,
                "select a.id, a.groupname, a.groupdesc "
                "from allgroup a inner join groupuser b on b.groupid = a.id "
                "where b.userid = %d",
                userid);
        MYSQL_RES *res = db.query(sql1);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setGroupName(row[1]);
                group.setGroupDesc(row[2]);

                char sql2[1024] = {0};
                sprintf(sql2,
                        "select a.id, a.name, a.state, b.grouprole "
                        "from user a inner join groupuser b on a.id = b.userid "
                        "where b.groupid = %d",
                        group.getId());
                MYSQL_RES *res2 = db.query(sql2);
                if (res2 != nullptr)
                {
                    MYSQL_ROW row2;
                    while ((row2 = mysql_fetch_row(res2)) != nullptr)
                    {
                        GroupUser user;
                        user.setId(atoi(row2[0]));
                        user.setName(row2[1]);
                        user.setState(row2[2]);
                        user.setRole(row2[3]);
                        group.getUsers().push_back(user);
                    }
                    mysql_free_result(res2);
                }
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

std::vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    std::vector<int> idVec;
    Db db;
    if (db.connect())
    {
        char sql[1024] = {0};
        sprintf(sql,
                "select userid from groupuser "
                "where groupid = %d and userid != %d",
                groupid,
                userid);
        MYSQL_RES *res = db.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}
