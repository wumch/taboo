
#pragma once

#include "predef.hpp"
#include <map>
#include <boost/shared_ptr.hpp>
#include "Config.hpp"

namespace taboo {

class BaseHandler
{
protected:
    typedef std::map<std::string, std::string, std::less<std::string> > ParamMap;

    typedef boost::shared_ptr<std::string> Response;

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

    virtual Response process()= 0;

    virtual ~BaseHandler() {}
};

}