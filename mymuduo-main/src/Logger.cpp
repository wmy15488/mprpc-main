#include"Logger.h"

    Logger& Logger::getinstance()
    {
          static Logger log;
          return log;
    }


    void Logger::log(std::string msg)
    {
           switch(loglevel_)
           {
            case INFO:
            std::cout<<"[INFO]";
            break;
            case ERORR:
            std::cout<<"[ERORR]";
            break;
            case FATAL:
            std::cout<<"[FATAL]";
            break;
            case DBUG:
            std::cout<<"[DBUG]";
            break;
            default:
            break;
           }
           std::cout<<msg<<std::endl;
    }


    void Logger::setlevel(int level)
    {
           loglevel_=level;
    }