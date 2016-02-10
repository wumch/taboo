
#pragma once

#include "../BaseHandler.hpp"
#include <functional>
#include <utility>
#include <map>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "rapidjson/document.h"
#include "../Config.hpp"
#include "../Aside.hpp"
#include "../Item.hpp"
#include "Signer.hpp"

namespace taboo  {
namespace manager {

class BaseHandler:
    public taboo::BaseHandler
{
protected:
    enum {
        err_bad_request_method  = 200001,
        err_bad_request         = 200002,
        err_bad_sign            = 200001,
        err_bad_param           = 200002,
    };

    typedef boost::unordered_map<ec_t, ReplyPtr> ReplyPtrMap;

    const ReplyPtr errUnknownReply;

    const ReplyPtrMap replys;

    std::string sign;

public:
    BaseHandler():
        taboo::BaseHandler(),
        errUnknownReply(genReply(err_unknown, "unknown error", mem_mode_persist))
    {
        initReplys();
    }

    virtual ReplyPtr process()
    {
        if (validate()) {
            prepare();
            ReplyPtr reply = _process();
            CS_DUMP(reply->content);
            return reply;
        } else {
            return errUnknownReply;
        }
    }

    virtual ~BaseHandler() {}

protected:
    virtual bool addParam(const char* key, const char* value, const std::size_t valueLength)
    {
        CS_DUMP(key);
        CS_DUMP(value);
        if (key == Config::instance()->keySign) {
            sign = std::string(value, valueLength);
        } else {
            params.insert(std::make_pair(std::string(key), std::string(value, valueLength)));
        }
        CS_SAY("a");
        return true;
    }

    virtual bool validate()
    {
        return checkParams() && checkSign();
    }

    virtual void prepare()
    {
        if (config->checkSign) {
            sign.clear();
        }
        params.clear();
    }

    virtual ReplyPtr _process()
    {
        ResPtr reply = deal();
        return reply ? (reply->reply ? reply->reply : getResponse(reply->code))
            : getResponse(err_unknown);
    }

    virtual ResPtr deal() const = 0;

    bool checkSign() const
    {
        if (!config->checkSign) {
            return true;
        }
        MD5Stream stream;
        stream << uri << config->signDelimiter
            << method << config->signDelimiter
            << config->manageSecret;
        for (ParamMap::const_iterator it = params.begin(); it != params.end(); ++it) {
            stream << config->signDelimiter << it->first << config->signHyphen << it->second;
        }
        return stream.hex() == sign;
    }

    virtual bool checkParams() const
    {
        return !sign.empty();
    }

    virtual ReplyPtr getResponse(ec_t errCode) const
    {
        ReplyPtrMap::const_iterator it = replys.find(errCode);
        if (it == replys.end()) {
            return errUnknownReply;
        } else {
            return it->second;
        }
    }

    virtual void initReplys()
    {
        ReplyPtrMap& map = const_cast<ReplyPtrMap&>(replys);
        map.insert(std::make_pair<ec_t, ReplyPtr>(err_unknown, errUnknownReply));
        fillReply(err_bad_request_method, "request method not allowed");
        fillReply(err_bad_request, "bad request");
        fillReply(err_bad_sign, "bad \"s\"i\"agn");
        fillReply(err_bad_param, "bad param");
        _initReplys(map);
    }

    virtual void fillReply(ec_t errCode, const std::string& errDesc)
    {
        const_cast<ReplyPtrMap&>(replys).insert(std::make_pair(errCode,
            genReply(errCode, errDesc, mem_mode_persist)));
    }

    virtual void _initReplys(ReplyPtrMap& _replys) {}
};

}
}
