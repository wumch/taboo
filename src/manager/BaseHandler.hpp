
#pragma once

#include "../predef.hpp"
#include <functional>
#include <utility>
#include <map>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include "rapidjson/document.h"
#include "../Config.hpp"
#include "../Aside.hpp"
#include "../BaseHandler.hpp"
#include "Signer.hpp"

namespace taboo  {
namespace manager {

class BaseHandler:
    public taboo::BaseHandler
{
protected:
    typedef int32_t ec_t;   // error-code type
    typedef boost::unordered_map<ec_t, Response> ResponseMap;

    typedef enum {
        err_err_ok = 0,
        err_unknown = 1,
        bad_request_method = 101,
        bad_request = 102,
        bad_sign = 201,
        bad_param = 202,
    };

    struct Res {
        ec_t code;
        std::string response;
    };
    typedef boost::shared_ptr<Res> ResPtr;

    const Response escapedQuotation;

    const Response errUnknownResponse;

    const ResponseMap responseMap;

public:
    BaseHandler():
        taboo::BaseHandler(), escapedQuotation(new std::string("\\\"")),
        errUnknownResponse(genResponse(error_unknown, "unknown error"))
    {
        initResponseMap();
    }

    virtual std::string process()
    {
        if (validate()) {
            prepare();
            return _process();
        } else {
            return errUnknownResponse;
        }
    }

    virtual bool addParam(const char* key, const char* value, const std::size_t valueLength)
    {
        params.insert(std::make_pair(std::string(key), std::string(value, valueLength)));
    }

    virtual ~BaseHandler() {}

protected:
    bool initResponseMap()
    {
        ResponseMap& resps = const_cast<ResponseMap&>(responseMap);
        resps.insert(std::make_pair(err_unknown, errUnknownResponse));
        resps.insert(std::make_pair(err_bad_request_method, genResponse(err_bad_request_method, "request method not allowed")));
        resps.insert(std::make_pair(err_bad_request, genResponse(err_bad_request, "bad request")));
        resps.insert(std::make_pair(err_bad_sign, genResponse(err_bad_sign, "bad sign")));
        resps.insert(std::make_pair(err_bad_param, genResponse(err_bad_param, "bad param")));
    }

    virtual bool validate()
    {
        return true;
    }

    virtual void prepare()
    {
        params.clear();
    }

    virtual Response _process() const = 0
    {
        ResPtr res = deal();
        if (res->response.empty()) {
            return getResponse(res->code);
        } else {
            return res->response;
        }
    }

    virtual ResPtr deal() const = 0;

    virtual const Response getResponse(ec_t errCode) const
    {
        ResponseMap::const_iterator it = responseMap.find(errCode);
        if (it == responseMap.end()) {
            return errUnknownResponse;
        } else {
            return it->second;
        }
    }

    Response genResponse(ec_t errCode, const std::string& errDesc) const
    {
        Response resp(new std::string);
        std::string& res = *resp;
        res.reserve(9 + 10 + Config::instance()->keyErrCode.length()
            + Config::instance()->keyErrDesc.length() + errDesc.length());
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
        return resp;
    }

    std::string quote(const std::string& str) const
    {
        std::string res = str;
        std::string::size_type pos = 0;
        while (pos = res.find('"', pos) != res.npos) {
            res = res.replace(pos, 1, escapedQuotation);
            pos += escapedQuotation.length();
        }
        return res;
    }
};

}
}