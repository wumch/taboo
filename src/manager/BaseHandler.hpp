
#pragma once

#include "../predef.hpp"
#include <functional>
#include <utility>
#include <map>
#include <sstream>
#include "rapidjson/document.h"
#include "../BaseHandler.hpp"
#include "../Config.hpp"
#include "../Aside.hpp"
#include "../Item.hpp"
#include "../Signer.hpp"

namespace taboo  {

class Router;

namespace manager {

template<ec_t handlerId>
class ManagerECAlloctor:
    protected taboo::ECAlloctor<1, handlerId>
{};

class BaseHandler:
    public taboo::BaseHandler,
    private ManagerECAlloctor<0>
{
    friend class taboo::Router;
    using ManagerECAlloctor<0>::ECA;
protected:
    enum {
        err_bad_request_method  = ECA::ECC<1>::value,
        err_bad_request         = ECA::ECC<2>::value,
        err_too_many_params     = ECA::ECC<3>::value,
        err_bad_param           = ECA::ECC<4>::value,
        err_bad_sign            = ECA::ECC<5>::value,
    };

    std::string sign;

public:
    BaseHandler():
        taboo::BaseHandler()
    {}

    virtual SharedReply process()
    {
        ec_t code = validate();
        if (code == err_ok) {
            return _process();
        } else {
            return getReply(code);
        }
    }

    static void initReplys()
    {
        fillReply(err_bad_request_method, "request method not allowed");
        fillReply(err_bad_request, "bad request");
        fillReply(err_bad_sign, "bad sign");
        fillReply(err_bad_param, "bad param");
    }

    virtual ~BaseHandler() {}

protected:
    virtual bool reviewParam(const std::string& key, const std::string& value)
    {
        if (key == config->keyMUSign) {
            sign = value;
            return false;
        }
        return _reviewParam(key, value);
    }

    virtual bool _reviewParam(const std::string& key, const std::string& value)
    {
        return true;
    }

    virtual ec_t validate()
    {
        ec_t code = checkParams();
        return (code == err_ok) ? checkSign() : code;
    }

    virtual SharedReply _process()
    {
        SharedResult res = deal();
        return res ? (res->reply ? res->reply : getReply(res->code))
            : getReply(err_unknown);
    }

    virtual SharedResult deal() const = 0;

    ec_t checkSign() const
    {
        if (!config->checkSign) {
            return err_ok;
        }
        MD5Stream stream;
        stream << uri << config->signDelimiter
            << method << config->signDelimiter
            << config->manageSecret;
        for (ParamMap::const_iterator it = params.begin(); it != params.end(); ++it) {
            stream << config->signDelimiter << it->first << config->signHyphen << it->second;
        }
        return stream.hex() == sign ? err_ok : err_bad_sign;
    }

    virtual ec_t checkParams() const
    {
        return config->checkSign && sign.empty() ? err_bad_param : err_ok;
    }

    virtual SharedReply getReply(ec_t errCode) const
    {
        CS_DUMP(errCode);
        CS_DUMP(replys.size());
        SharedReplyMap::const_iterator it = replys.find(errCode);
        if (it == replys.end()) {
            return errUnknownReply;
        } else {
            return it->second;
        }
    }
};

}
}
