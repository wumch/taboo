
#include "Router.hpp"
#include "manager/StatusHandler.hpp"
#include "manager/AttachHandler.hpp"
#include "manager/DetachHandler.hpp"

namespace taboo {

Router* Router::_instance = new Router;

void Router::initHandlerMap()
{
    HandlerCreatorMap& creators = const_cast<HandlerCreatorMap&>(handlerCreatorMap);
    creators.insert(std::make_pair(std::string("/manage/status"), &manager::StatusHandler::create));
    // params: update=1
    creators.insert(std::make_pair(std::string("/manage/attach"), &manager::AttachHandler::create));

    creators.insert(std::make_pair(std::string("/manage/detach"), manager::DetachHandler::create));

    creators.insert(std::make_pair(std::string("/manage/erase"), &manager::DetachHandler::create));

    // force=0
//    creators.insert(std::make_pair(std::string("/manage/tidy"), &manager::DetachHandler::create));

      // params: force=0
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
