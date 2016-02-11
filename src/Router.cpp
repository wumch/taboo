
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

const std::string BaseHandler::escapedQuotation("\\\"");
const std::string BaseHandler::methodGet("GET");
const std::string BaseHandler::methodPost("POST");

const SharedReply BaseHandler::errUnknownReply;

const BaseHandler::SharedReplyMap BaseHandler::replys;

const SharedReply manager::AttachHandler::okReply;

void Router::initHandlerRelyMap()
{
    manager::BaseHandler::initReplys();
    manager::StatusHandler::initReplys();
    manager::AttachHandler::initReplys();
    manager::DetachHandler::initReplys();
}

}
