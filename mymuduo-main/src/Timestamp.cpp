#include"Timestamp.h"

#include<time.h>

Timestamp::Timestamp():t_(0)
{

}
Timestamp::Timestamp(int64_t t)
{
  t_=t;
}

Timestamp Timestamp::now()
{
 return Timestamp(time(NULL));
}

std::string Timestamp::tostring()const
{
  char buf[128]={0};
  tm* tm=localtime(&t_);
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
        tm->tm_year + 1900,
        tm->tm_mon + 1,
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec);
  return buf;
}