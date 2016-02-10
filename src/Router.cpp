
#include "Router.hpp"
#include "manager/StatusHandler.hpp"
#include "manager/AttachHandler.hpp"
#include "manager/DetachHandler.hpp"

namespace taboo
{

Router* Router::_instance = new Router;

void Router::initHandlerMap()
{
    HandlerMap& handlers = const_cast<HandlerMap&>(handlerMap);
    handlers.insert(std::make_pair(std::string("/manage/status"), HandlerPtr(new manager::StatusHandler)));
    handlers.insert(std::make_pair(std::string("/manage/attach"), HandlerPtr(new manager::AttachHandler)));
    handlers.insert(std::make_pair(std::string("/manage/detach"), HandlerPtr(new manager::DetachHandler)));
    handlers.insert(std::make_pair(std::string("/manage/erase"), HandlerPtr(new manager::DetachHandler)));
}

}
