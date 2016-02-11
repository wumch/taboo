
#pragma once

#include "../BaseHandler.hpp"
#include <functional>
#include <utility>
#include <map>
#include <sstream>
#include <boost/unordered_map.hpp>
#include "rapidjson/document.h"
#include "../Config.hpp"
#include "../Aside.hpp"
#include "../Item.hpp"
#include "Signer.hpp"

namespace taboo  {

class Router;

namespace manager {

class BaseHandler:
    public taboo::BaseHandler
{
    friend class taboo::Router;
protected:
    enum {
        err_bad_request_method  = 200001,
        err_bad_request         = 200002,
        err_bad_param           = 200003,
        err_bad_sign            = 200004,
    };

    typedef boost::unordered_map<ec_t, ReplyPtr> ReplyPtrMap;

    static const ReplyPtr errUnknownReply;

    static const ReplyPtrMap replys;

    std::string sign;

public:
    BaseHandler():
        taboo::BaseHandler()
    {}

    virtual ReplyPtr process()
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
        const_cast<ReplyPtr&>(errUnknownReply) =
            genReply(err_unknown, "unknown error", mem_mode_persist);
        ReplyPtrMap& map = const_cast<ReplyPtrMap&>(replys);
        map.insert(std::make_pair<ec_t, ReplyPtr>(err_unknown, errUnknownReply));
        fillReply(err_bad_request_method, "request method not allowed");
        fillReply(err_bad_request, "bad request");
        fillReply(err_bad_sign, "bad sign");
        fillReply(err_bad_param, "bad param");
    }

    virtual ~BaseHandler() {}

protected:
    virtual bool addParam(const char* key, const char* value, const std::size_t valueLength)
    {
        if (key == Config::instance()->keySign) {
            sign = std::string(value, valueLength);
        } else {
            params.insert(std::make_pair(std::string(key), std::string(value, valueLength)));
        }
        return true;
    }

    virtual ec_t validate()
    {
        ec_t code = checkParams();
        CS_DUMP(code);
        return (code == err_ok) ? checkSign() : code;
    }

    virtual ReplyPtr _process()
    {
        ResPtr res = deal();
        return res ? (res->reply ? res->reply : getReply(res->code))
            : getReply(err_unknown);
    }

    virtual ResPtr deal() const = 0;

    ec_t checkSign() const
    {
        if (!config->checkSign) {
            return err_bad_sign;
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
        return sign.empty() ? err_bad_param : err_ok;
    }

    virtual ReplyPtr getReply(ec_t errCode) const
    {
        CS_DUMP(errCode);
        CS_DUMP(replys.size());
        ReplyPtrMap::const_iterator it = replys.find(errCode);
        if (it == replys.end()) {
            return errUnknownReply;
        } else {
            return it->second;
        }
    }

    static void fillReply(ec_t errCode, const std::string& errDesc)
    {
        CS_SAY(errCode << ": " << errDesc);
        const_cast<ReplyPtrMap&>(replys).insert(std::make_pair(errCode,
            genReply(errCode, errDesc, mem_mode_persist)));
    }
};

}
}
