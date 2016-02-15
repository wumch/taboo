
#pragma once

#include "predef.hpp"
#include <memory>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
extern "C" {
#   include <microhttpd.h>
}
#include "Config.hpp"

namespace taboo {

typedef enum {
    mem_mode_persist        = MHD_RESPMEM_PERSISTENT,
    mem_mode_must_free      = MHD_RESPMEM_MUST_FREE,
    mem_mode_must_copy      = MHD_RESPMEM_MUST_COPY,
} MemMode;

class Reply
{
public:
    std::string content;
    MemMode memMode;

    explicit Reply(const std::string& _content):
        content(_content), memMode(mem_mode_must_copy)
    {}

    explicit Reply(MemMode _memMode):
        memMode(mem_mode_must_copy)
    {}

    Reply(const std::string& _content, MemMode _memMode):
        content(_content), memMode(_memMode)
    {}
};

typedef boost::shared_ptr<Reply> SharedReply;

typedef int32_t ec_t;   // error-code type

template<ec_t moduleId, ec_t handlerId>
class ECAlloctor
{
protected:
    typedef ECAlloctor<moduleId, handlerId> ECA;

    template<ec_t ecId>
    class ECC   // error code calculater
    {
    public:
        enum { value = moduleId * 100000 + handlerId * 1000 + ecId };
    };
};

class Router;

class BaseHandler:
    private ECAlloctor<0, 0>
{
    friend class Router;
private:
    static const std::string escapedQuotation;

protected:
    enum {
        err_ok              = ECA::ECC<0>::value,
        err_unknown         = ECA::ECC<1>::value,
        err_no_route        = ECA::ECC<2>::value,
        err_process_failed  = ECA::ECC<3>::value,
    };
    typedef std::map<std::string, std::string, std::less<std::string> > ParamMap;

    struct Result {
    public:
        SharedReply reply;
        ec_t code;
        Result():
            code(err_unknown)
        {}

        explicit Result(ec_t errCode):
            code(errCode)
        {}
    };
    typedef boost::shared_ptr<Result> SharedResult;

    static  const std::string methodGet, methodPost, methodOptions;

    static const SharedReply errUnknownReply;

    typedef boost::unordered_map<ec_t, SharedReply> SharedReplyMap;
    static const SharedReplyMap replys;

    const Config* config;

    std::string method, uri;

    ParamMap params;

public:
    BaseHandler():
        config(Config::instance())
    {}

    virtual bool setMeta(const std::string& _method, const std::string& _uri)
    {
        if (CS_BUNLIKELY(_method.empty() || _uri.empty())) {
            return false;
        }
        method = _method;
        if (*_uri.rbegin() == '/') {
            uri = _uri.substr(0, _uri.length() - 1);
        } else {
            uri = _uri;
        }
        return true;
    }

    bool isPost() const
    {
        return method == methodPost;
    }

    virtual bool addGetParam(const char* key, const char* value)
    {
        return addParam(key, value);
    }

    virtual bool addPostParam(const char* key, const char* value, std::size_t valueLength)
    {
        return addParam(key, std::string(value, valueLength));
    }

    virtual SharedReply process() = 0;

    virtual ~BaseHandler() {}

protected:
    virtual bool addParam(const std::string& key, const std::string& value)
    {
        if (reviewParam(key, value)) {
            params.insert(std::make_pair(key, value));
        }
        return true;
    }

    virtual bool reviewParam(const std::string& key, const std::string& value)
    {
        return true;
    }

    static void initReplys()
    {
        const_cast<SharedReply&>(errUnknownReply) =
            genReply(err_unknown, "unknown error", mem_mode_persist);
        const_cast<SharedReplyMap&>(replys).insert(
            std::make_pair<ec_t, SharedReply>(err_unknown, errUnknownReply));
        fillReply(err_process_failed, "failed on processing");
    }

    static SharedReply genReply(ec_t errCode, const std::string& errDesc,
        MemMode memMode = mem_mode_must_copy)
    {
        SharedReply reply(new Reply(memMode));
        std::string& content = reply->content;
        content.reserve((9 + 10) + Config::instance()->keyMDErrCode.length()
            + Config::instance()->keyMDErrDesc.length() + errDesc.length());
        content += "{\"";
        content += Config::instance()->keyMDErrCode;
        content += "\":";
        content += boost::lexical_cast<std::string>(errCode);
        if (!errDesc.empty()) {
            content += ",\"";
            content += Config::instance()->keyMDErrDesc;
            content += "\":\"";
            if (errDesc.find('"') == errDesc.npos) {
                content += errDesc;
            } else {
                content += quote(errDesc);
            }
            content += '"';
        }
        content += '}';
        CS_DUMP(content);
        return reply;
    }

    static void fillReply(ec_t errCode, const std::string& errDesc)
    {
        CS_SAY(errCode << ": " << errDesc);
        const_cast<SharedReplyMap&>(replys).insert(std::make_pair(errCode,
            genReply(errCode, errDesc, mem_mode_persist)));
    }

    static std::string quote(const std::string& str)
    {
        std::string res;
        res.reserve(str.length() + 32);
        std::string::size_type consumed = 0, pos = 0;
        while ((pos = str.find('"', pos)) != str.npos) {
            res.append(str, consumed, pos - consumed);
            res += escapedQuotation;
            consumed = ++pos;
        }
        if (str.length() != consumed) {
            res.append(str, consumed, str.length() - consumed);
        }
        return res;
    }
};

typedef std::auto_ptr<BaseHandler> SharedHandler;

template<typename HandlerType>
class HandlerCreator
{
public:
    static SharedHandler create()
    {
        SharedHandler handler(new HandlerType);
        return handler;
    }
};

class NoRouteHandler:
    public BaseHandler,
    public HandlerCreator<NoRouteHandler>
{
private:
    static const SharedReply reply;

public:
    virtual SharedReply process()
    {
        return reply;
    }

    static void initReplys()
    {
        const_cast<SharedReply&>(reply) = genReply(err_no_route, "no matching route", mem_mode_persist);
    }

protected:
    virtual bool addParam(const std::string& key, const std::string& value)
    {
        return true;
    }
};

}
