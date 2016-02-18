
#pragma once

#include "../predef.hpp"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "BasePredicter.hpp"

typedef websocketpp::server<websocketpp::config::asio> Server;

namespace taboo {
namespace query {

class WsPredicter :
    public BasePredicter,
    private QueryECAlloctor<1>
{
    friend class taboo::Router;
    using QueryECAlloctor<1>::ECA;
protected:
    Server::message_ptr message;

public:
    typedef boost::shared_ptr<query::WsPredicter> SharedWsPredicter;

    virtual SharedReply process()
    {
        CS_SAY("predicting");
        if (message->get_payload().empty()) {
            CS_SAY("no query");
            return errNoQueryReply;
        }
        if (message->get_payload().length() > config->queryDataMaxSize) {
            CS_SAY("query data too long");
            return errBadQueryReply;
        }
        if (!query.rebuild(message->get_payload())) {
            CS_SAY("bad query");
            return errBadQueryReply;
        }
        return SharedReply(new Reply(predict(), mem_mode_must_copy));
    }

    void setMessage(Server::message_ptr _message)
    {
        message = _message;
    }

    static SharedWsPredicter create()
    {
        SharedWsPredicter predicter(new WsPredicter);
        return predicter;
    }

protected:
    static void initReplys() {}
};

}

typedef query::WsPredicter::SharedWsPredicter SharedWsPredicter;

}
