
#pragma once

#include "predef.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include "Config.hpp"
#include "BaseHandler.hpp"
#include "query/WsPredicter.hpp"

namespace taboo {

typedef SharedHandler (*HandlerCreatorFunc)();

class Router
{
private:
    static Router* _instance;

    typedef boost::unordered_map<std::string, HandlerCreatorFunc> HandlerCreatorMap;
    HandlerCreatorMap handlerCreatorMap;
    HandlerCreatorFunc noRouteHandlerCreator;

    Router():
        noRouteHandlerCreator(&NoRouteHandler::create)
    {
        initHandlerMap();
    }

public:
    static Router* instance()
    {
        return _instance;
    }

    static bool initialize()
    {
        initHandlerRelyMap();
        return true;
    }

    SharedHandler route(const std::string& method, const std::string& uri) const
    {
        if (uri.empty()) {
            return noRouteHandlerCreator();
        }
        HandlerCreatorMap::const_iterator it = handlerCreatorMap.find(uri);
        if (it == handlerCreatorMap.end()) {
            return noRouteHandlerCreator();
        }
        SharedHandler handler = (it->second)();
        handler->setMeta(method, uri);
        return handler;
    }

    SharedWsPredicter route(Server::message_ptr message) const;

protected:
    void initHandlerMap();

    static void initHandlerRelyMap();
};

}
