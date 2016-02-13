
#pragma once

#include "../predef.hpp"
#include "../BaseHandler.hpp"
#include "BasePredicter.hpp"

namespace taboo {
namespace query {

class PredictHandler:
    public taboo::BaseHandler,
    public BasePredicter,
    public taboo::HandlerCreator<PredictHandler>,
    private QueryECAlloctor<1>
{
    friend class taboo::Router;
    using QueryECAlloctor<1>::ECA;
protected:
    enum {
        err_no_query        = ECA::ECC<1>::value,
        err_bad_query       = ECA::ECC<2>::value,
    };

    static const SharedReply errNoQueryReply;
    static const SharedReply errBadQueryReply;

    mutable Query query;

public:
    virtual SharedReply process()
    {
        CS_SAY("predicting");
        ParamMap::const_iterator it = params.find(config->keyQuery);
        if (it == params.end()) {
            CS_SAY("no query");
            return errNoQueryReply;
        }
        if (!Query::rebuild(query, it->second)) {
            CS_SAY("bad query");
            return errBadQueryReply;
        }
        std::string res = predict(query);
        CS_DUMP(res);
        return genReply(err_ok, res);
    }

    static void initReplys()
    {
        const_cast<SharedReply&>(errNoQueryReply) =
            genReply(err_no_query, "no '" + Config::instance()->keyQuery + "' specified");
        const_cast<SharedReply&>(errBadQueryReply) =
            genReply(err_bad_query, "bad '" + Config::instance()->keyQuery + "' specified");
    }
};

}
}
