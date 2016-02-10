
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
    typedef boost::shared_ptr<BaseHandler> HandlerPtr;
    typedef boost::unordered_map<std::string, HandlerPtr> HandlerMap;

    enum {
        err_no_route = -1,
    };

    const Config* config;

    const HandlerMap handlerMap;

    const std::string noRouteResponse;

public:
    Router():
        noRouteResponse("{\"" + config->keyErrCode + "\":"
            + boost::lexical_cast<std::string>(err_no_route) + "\"}")
    {
        initHandlerMap();
    }

protected:
    void initHandlerMap();
};

}
