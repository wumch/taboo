
#pragma once

#include "predef.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include "Config.hpp"
#include "BaseHandler.hpp"

namespace taboo {

typedef HandlerPtr (*HandlerCreatorFunc)();

class Router
{
private:
    static Router* _instance;

    typedef boost::unordered_map<std::string, HandlerCreatorFunc> HandlerMap;

    enum {
        err_no_route = 101,
    };

    const Config* config;

    HandlerMap handlerMap;

    HandlerCreatorFunc noRouteHandlerCreator;

    Router():
        config(Config::instance()),
        noRouteHandlerCreator(&NoRouteHandler::create)
    {
        initHandlerMap();
    }

public:
    static Router* instance()
    {
        return _instance;
    }

    HandlerPtr route(const std::string& method, const std::string& uri) const
    {
        CS_DUMP(method);
        CS_DUMP(uri);
        if (uri.empty()) {
            return noRouteHandlerCreator();
        }
        HandlerMap::const_iterator it = handlerMap.find(uri);
        if (it == handlerMap.end()) {
            CS_SAY("no route");
            return noRouteHandlerCreator();
        }
        CS_SAY("route succeed");
        HandlerPtr handler = (it->second)();
        handler->setMeta(method, uri);
        CS_DUMP(handler->isPost());
        return handler;
    }

protected:
    void initHandlerMap();
};

}
