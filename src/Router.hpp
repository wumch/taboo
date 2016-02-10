
#pragma once

#include "predef.hpp"
#include "manager/AttachHandler.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

namespace taboo {

class Router
{
private:
    typedef boost::shared_ptr<manager::BaseHandler> HandlerPtr;
    typedef boost::unordered_map<std::string, HandlerPtr> HandlerMap;

    HandlerMap handlerMap;

public:
    Router();

};

}
