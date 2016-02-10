
#include "Router.hpp"
#include "manager/StatusHandler.hpp"
#include "manager/AttachHandler.hpp"
#include "manager/DetachHandler.hpp"

namespace taboo {

Router* Router::_instance = new Router;

void Router::initHandlerMap()
{
    HandlerMap& handlers = const_cast<HandlerMap&>(handlerMap);
    handlers.insert(std::make_pair(std::string("/manage/status"), &manager::StatusHandler::create));
    handlers.insert(std::make_pair(std::string("/manage/attach"), &manager::AttachHandler::create));
    handlers.insert(std::make_pair(std::string("/manage/detach"), manager::DetachHandler::create));
    handlers.insert(std::make_pair(std::string("/manage/erase"), &manager::DetachHandler::create));
//    handlers.insert(std::make_pair(std::string("/manage/store"), &manager::StoreHandler::create));
}

}
