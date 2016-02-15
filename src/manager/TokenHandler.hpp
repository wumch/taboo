
#pragma once

#include "../predef.hpp"
#include "BaseHandler.hpp"
#include "../Session.hpp"

namespace taboo {
namespace manager {

class TokenHandler:
    public BaseHandler,
    public HandlerCreator<TokenHandler>,
    private ManagerECAlloctor<3>
{
    using ManagerECAlloctor<3>::ECA;
protected:
    mutable Value filters;

    virtual SharedResult deal() const
    {
        SharedResult res(new Result);
        do {
            if (filters.IsNull()) {
                res->code = err_bad_param;
                break;
            }
            identy_t identy;
            {
                ParamMap::const_iterator it = params.find(config->keyMUIdenty);
                if (it == params.end()) {
                    res->code = err_bad_param;
                    break;
                }
                identy = boost::lexical_cast<identy_t>(it->second);
            }
            std::time_t expire;
            {
                ParamMap::const_iterator it = params.find(config->keyMUExpire);
                expire = (it == params.end())
                    ? std::time(NULL) + config->tokenDefaultExpire
                    : boost::lexical_cast<std::time_t>(it->second);
            }
            std::string token = SessionManager::instance()->ensure(identy, filters, expire);
            res->reply = genReply(err_ok, token, mem_mode_must_copy);
        } while (false);
        return res;
    }

    virtual bool _reviewParam(const std::string& key, const std::string& value)
    {
        if (key == config->keyMUFilters) {
            Dom dom;
            dom.Parse(value.c_str());
            if (dom.HasParseError() || dom.IsObject()) {
                return false;
            }
            filters.Swap(dom);
        }
        return true;
    }
};

}
}
