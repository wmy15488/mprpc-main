
#include"nocopyable.h"
#include<string.h>
#include<iostream>

enum LogLevel
{
    INFO,
    ERORR,
    FATAL,
    DBUG,
};

#define  LOG_INFO(msg,...)\
do\
{\
   Logger&logger=Logger::getinstance();\
   logger.setlevel(INFO);\
   char buf[1024]={0};\
   snprintf(buf,1024,msg,##__VA_ARGS__);\
   logger.log(buf);\
\
}while(0)

#define  LOG_ERORR(msg,...)\
do\
{\
   Logger&logger=Logger::getinstance();\
   logger.setlevel(ERORR);\
   char buf[1024]={0};\
   snprintf(buf,1024,msg,##__VA_ARGS__);\
   logger.log(buf);\
\
}while(0)

#define  LOG_FATAL(msg,...)\
do\
{\
   Logger&logger=Logger::getinstance();\
   logger.setlevel(FATAL);\
   char buf[1024]={0};\
   snprintf(buf,1024,msg,##__VA_ARGS__);\
   logger.log(buf);\
\
}while(0)

#define  LOG_DBUG(msg,...)\
do\
{\
   Logger&logger=Logger::getinstance();\
   logger.setlevel(DBUG);\
   char buf[1024]={0};\
   snprintf(buf,1024,msg,##__VA_ARGS__);\
   logger.log(buf);\
\
}while(0)

class Logger:nocopyable
{
    public:
    static Logger&getinstance();
    void log(std::string msg);
    void setlevel(int level);

    private:
    int loglevel_;

    Logger(){}
};