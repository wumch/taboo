
#pragma once

#include "../predef.hpp"
#include "BasePredicter.hpp"

namespace taboo {
namespace query {

class PredictHandler:
    public BasePredicter,
    public taboo::HandlerCreator<PredictHandler>,
    private QueryECAlloctor<1>
{
    friend class taboo::Router;
    using QueryECAlloctor<1>::ECA;
public:
    virtual SharedReply process()
    {
        CS_SAY("predicting");
        ParamMap::const_iterator it = params.find(config->keyQUPayload);
        if (it == params.end()) {
            CS_SAY("no query");
            return errNoQueryReply;
        }
        if (it->second.length() > config->queryDataMaxSize) {
            CS_SAY("query data too long");
            return errBadQueryReply;
        }
        if (!query.rebuild(it->second)) {
            CS_SAY("bad query");
            return errBadQueryReply;
        }
        return SharedReply(new Reply(predict(), mem_mode_must_copy));
    }
};

}
}
