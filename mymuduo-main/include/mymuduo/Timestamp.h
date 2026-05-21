#pragma once
#include<iostream>
#include<string>


class Timestamp
{
    public:
    Timestamp();
    Timestamp(int64_t t);
    static Timestamp now();
    std::string tostring()const;
    private:
    int64_t t_;
};