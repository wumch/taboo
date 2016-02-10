
#pragma once

#include "predef.hpp"
#include "Config.hpp"

namespace taboo {

class BaseHandler
{
protected:
    const Config* config;

    const std::string methodGet, methdoPost;

    ParamMap params;

public:
    BaseHandler():
        config(Config::instance()),
        methdoGet("GET"), methodPost("POST")
    {}

    virtual bool addParam(const char* key, const char* value, std::size_t valueLength)
    {
        params.insert(std::make_pair(std::string(key), std::string(value, valueLength)));
    }

    virtual std::string process()= 0;

    virtual ~BaseHandler() {}
};

}