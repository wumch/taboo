
#pragma once

#include "predef.hpp"
#include <memory>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
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

typedef boost::shared_ptr<Reply> ReplyPtr;

class BaseHandler
{
private:
    const std::string escapedQuotation;

protected:
    enum {
        err_ok          = 0,
        err_unknown     = 1,
        err_no_route    = 1001,
    };
    typedef int32_t ec_t;   // error-code type
    typedef std::map<std::string, std::string, std::less<std::string> > ParamMap;

    struct Res {
    public:
        ReplyPtr reply;
        ec_t code;
        Res():
            code(err_unknown)
        {}
    };
    typedef boost::shared_ptr<Res> ResPtr;

    const Config* config;

    const std::string methodGet, methodPost;

    std::string method, uri;

    ParamMap params;

public:
    BaseHandler():
        config(Config::instance()), escapedQuotation("\\\""),
        methodGet("GET"), methodPost("POST")
    {}

    virtual bool setMeta(const std::string& _method, const std::string& _uri)
    {
        if (CS_BUNLIKELY(method.empty() || uri.empty())) {
            return false;
        }
        method = _method;
        if (*uri.rbegin() == '/') {
            uri = _uri.substr(0, _uri.length() - 1);
        } else {
            uri = _uri;
        }
        return true;
    }

    bool isPost() const     // todo: wired with WebSocket requests
    {
        return method == methodPost;
    }

    virtual bool addGetParam(const char* key, const char* value)
    {
        return addParam(key, value, std::strlen(value));
    }

    virtual bool addPostParam(const char* key, const char* value, std::size_t valueLength)
    {
        return addParam(key, value, valueLength);
    }

    virtual ReplyPtr process() = 0;

    virtual ~BaseHandler() {}

protected:
    virtual bool addParam(const char* key, const char* value, std::size_t valueLength)
    {
        params.insert(std::make_pair(std::string(key), std::string(value, valueLength)));
        return true;
    }

    std::string quote(const std::string& str) const
    {
        std::string res;
        res.reserve(str.length() + 32);
        std::string::size_type consumed = 0, pos = 0;
        while ((pos = res.find('"', pos)) != res.npos) {
            res.append(str, consumed, pos);
            res += escapedQuotation;
            consumed = ++pos;
        }
        if (str.length() != consumed) {
            res.append(str, consumed, str.length() - consumed);
        }
        return res;
    }

    ReplyPtr genReply(ec_t errCode, const std::string& errDesc,
        MemMode memMode = mem_mode_must_copy) const
    {
        ReplyPtr reply(new Reply(memMode));
        std::string& res = reply->content;
        res.reserve(9 + 10 + config->keyErrCode.length()
            + config->keyErrDesc.length() + errDesc.length());
        res += "{\"";
        res += Aside::instance()->keyErrCode;
        res += "\":";
        res += boost::lexical_cast<std::string>(errCode);
        if (!errDesc.empty()) {
            res += ",\"";
            res += Aside::instance()->keyErrDesc;
            res += "\":\"";
            if (errDesc.find('"') == errDesc.npos) {
                res += errDesc;
            } else {
                res += quote(errDesc);
            }
        }
        res += "\"}";
        return reply;
    }
};

typedef std::auto_ptr<BaseHandler> HandlerPtr;

template<typename HandlerType>
class HandlerCreator
{
public:
    static HandlerPtr create()
    {
        HandlerPtr handler(new HandlerType);
        return handler;
    }
};

class NoRouteHandler:
    public BaseHandler
{
private:
    const ReplyPtr reply;

public:
    NoRouteHandler():
        reply(genReply(err_no_route, "{\"" + config->keyErrCode + "\":"
            + boost::lexical_cast<std::string>(err_no_route) + "\"}", mem_mode_persist))
    {}

    virtual ReplyPtr process()
    {
        return reply;
    }

protected:
    virtual bool addParam(const char* key, const char* value, std::size_t valueLength)
    {
        return true;
    }
};

}
