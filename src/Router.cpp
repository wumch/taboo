
#include "Router.hpp"
#include "manager/StatusHandler.hpp"
#include "manager/AttachHandler.hpp"
#include "manager/DetachHandler.hpp"
#include "query/PredictHandler.hpp"

namespace taboo {

Router* Router::_instance = new Router;

void Router::initHandlerMap()
{
    HandlerCreatorMap& creators = const_cast<HandlerCreatorMap&>(handlerCreatorMap);
    creators.insert(std::make_pair(std::string("/manage/status"), &manager::StatusHandler::create));
    // params: update=1
    creators.insert(std::make_pair(std::string("/manage/attach"), &manager::AttachHandler::create));

    creators.insert(std::make_pair(std::string("/manage/detach"), manager::DetachHandler::create));

    // todo: How to clean an item once all it's prefixes are lost ?
    creators.insert(std::make_pair(std::string("/manage/erase"), &manager::DetachHandler::create));

    creators.insert(std::make_pair(std::string("/manage/get_access_token"), &manager::DetachHandler::create));

    creators.insert(std::make_pair(std::string("/query/predict"), &query::PredictHandler::create));

    // force=0
//    creators.insert(std::make_pair(std::string("/manage/tidy"), &manager::DetachHandler::create));

    // params: force=0
//    handlers.insert(std::make_pair(std::string("/manage/store"), &manager::StoreHandler::create));
}

const std::string BaseHandler::escapedQuotation("\\\"");
const std::string BaseHandler::methodGet("GET");
const std::string BaseHandler::methodPost("POST");
const std::string BaseHandler::methodOptions("OPTIONS");

const SharedReply BaseHandler::errUnknownReply;

const SharedReply NoRouteHandler::reply;

const BaseHandler::SharedReplyMap BaseHandler::replys;

const SharedReply manager::AttachHandler::okReply;

const SharedReply query::BasePredicter::errNoQueryReply;
const SharedReply query::BasePredicter::errBadQueryReply;

void Router::initHandlerRelyMap()
{
    NoRouteHandler::initReplys();
    manager::BaseHandler::initReplys();
    manager::StatusHandler::initReplys();
    manager::AttachHandler::initReplys();
    manager::DetachHandler::initReplys();
    query::BasePredicter::initReplys();
}

}
