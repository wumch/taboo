
#include "Router.hpp"
#include "manager/AttachHandler.hpp"

namespace taboo
{

void Router::initHandlerMap()
{
    HandlerMap& handlers = const_cast<HandlerMap&>(handlerMap);
    handlers.insert(std::make_pair("/attach", HanderPtr(new manager::AttachHandler)));
}

}