
#pragma once

#ifndef _TOOLS_HPP_
#define _TOOLS_HPP_

#include <string>

class Database
{
public:
    virtual void get() = 0;
    virtual void set() = 0;
    virtual void lGet() = 0;
    virtual void lPush() = 0;
};

class Tic
{
public:
    Tic(std::string name) : name(name) {}
    void incr() { ++this->count; }
    void reset() { this->count=0; }
    int const &get() const { return this->count; }

private:
    int count;
    std::string name;
};

#endif // !_TOOLS_HPP_
