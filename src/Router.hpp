
#pragma once

#include "predef.hpp"
#include "manager/AttachHandler.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include "Config.hpp"
#include "BaseHandler.hpp"

namespace taboo {

class Router
{
private:
    static Router* _instance;

    typedef boost::shared_ptr<BaseHandler> HandlerPtr;
    typedef boost::unordered_map<std::string, HandlerPtr> HandlerMap;

    enum {
        err_no_route = 101,
    };

    const Config* config;

    HandlerMap handlerMap;

    const HandlerPtr noRouteHandler;

    static Router* instance()
    {
        return _instance;
    }

    Router():
        config(Config::instance()),
        noRouteHandler(new NoRouteHandler)
    {
        initHandlerMap();
    }

public:
    HandlerPtr route(const std::string& method, const std::string& uri) const
    {
        HandlerMap::const_iterator it;
        if (uri.empty()) {
            return noRouteHandler;
        }
        HandlerMap::const_iterator it = handlerMap.find(uri);
        if (it == handlerMap.end()) {
            return noRouteHandler;
        }
        return it->second;
    }

protected:
    void initHandlerMap();
};

}
