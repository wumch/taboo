
#pragma once

#include "../predef.hpp"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "../BaseHandler.hpp"
#include "../Query.hpp"
#include "../Seeker.hpp"

namespace taboo {

class Router;

namespace query {

template<ec_t handlerId>
class QueryECAlloctor:
    protected taboo::ECAlloctor<2, handlerId>
{};

class BasePredicter:
    public taboo::BaseHandler,
    private QueryECAlloctor<0>
{
    friend class taboo::Router;
    using QueryECAlloctor<0>::ECA;
protected:
    enum {
        err_bad_request_method      = ECA::ECC<1>::value,
        err_bad_request             = ECA::ECC<2>::value,
        err_too_many_params         = ECA::ECC<3>::value,
        err_bad_param               = ECA::ECC<4>::value,
        err_no_query                = ECA::ECC<5>::value,
        err_bad_query               = ECA::ECC<6>::value,
    };

    static const SharedReply errNoQueryReply;
    static const SharedReply errBadQueryReply;

    const std::string emptyResult;

    const Seeker seeker;

    mutable Query query;

public:
    BasePredicter() {}

    std::string predict() const
    {
        const SharedItemList& items = seeker.seek(query);
        return formReply(items, err_ok);
    }

    virtual ~BasePredicter() {};

protected:
    typedef rapidjson::Writer<rapidjson::StringBuffer> JsonWriter;

    std::string formReply(const SharedItemList& items, ec_t errCode) const
    {
        const Aside* const aside = Aside::instance();
        // todo: provide pre-allocted buffer to rapidjson::StringBuffer.
        rapidjson::StringBuffer buffer(0, config->querySendBuffer);
        JsonWriter writer(buffer);
        writer.StartObject();

        aside->keyQDErrCode.Accept(writer);
        writer.Uint(errCode);

        aside->keyQDPayload.Accept(writer);
        writer.StartArray();
        query.fieldsAll ? addItems(writer, items) : addItems(writer, items, query.fields);
        writer.EndArray();

        if (query.echoData) {
            aside->keyQEchoData.Accept(writer);
            query.echoData->Accept(writer);
        }

        writer.EndObject();

        return buffer.GetString();
    }

    void addItems(JsonWriter& writer, const SharedItemList& items, const ValuePtrSet& fields) const
    {
        for (SharedItemList::const_iterator item = items.begin(); item != items.end(); ++item) {
            const Dom& dom = (*item)->dom;
            writer.StartObject();
            for (ValuePtrSet::const_iterator key = fields.begin(); key != fields.end(); ++key) {
                Dom::ConstMemberIterator it = dom.FindMember(**key);
                if (it != dom.MemberEnd()) {
                    it->name.Accept(writer);
                    it->value.Accept(writer);
                }
            }
            writer.EndObject();
        }
    }

    void addItems(JsonWriter& writer, const SharedItemList& items) const
    {
        for (SharedItemList::const_iterator item = items.begin(); item != items.end(); ++item) {
            (*item)->dom.Accept(writer);
        }
    }

    static void initReplys()
    {
        const_cast<SharedReply&>(errNoQueryReply) =
            genReply(err_no_query, "no '" + Config::instance()->keyQUPayload + "' specified");
        const_cast<SharedReply&>(errBadQueryReply) =
            genReply(err_bad_query, "bad '" + Config::instance()->keyQUPayload + "' specified");
    }
};

}
}
