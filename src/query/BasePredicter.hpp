
#pragma once

#include "../predef.hpp"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
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
    };

    const std::string emptyResult;

    const Seeker seeker;

public:
    BasePredicter() {}

    std::string predict(const Query& query) const
    {
        const SharedItemList& items = seeker.seek(query);
        return query.fieldsAll ? formReply(items) : formReply(items, query.fields);
    }

    virtual ~BasePredicter() {};

protected:
    const char* formReply(const SharedItemList& items, const ValuePtrSet& fields) const
    {
        // todo: provide pre-allocted buffer to rapidjson::StringBuffer.
        rapidjson::StringBuffer buffer(0, Config::instance()->querySendBuffer);
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        writer.StartArray();
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
        writer.EndArray();
        return buffer.GetString();
    }

    const char* formReply(const SharedItemList& items) const
    {
        rapidjson::StringBuffer buffer(0, Config::instance()->querySendBuffer);
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        writer.StartArray();
        for (SharedItemList::const_iterator item = items.begin(); item != items.end(); ++item) {
            (*item)->dom.Accept(writer);
        }
        writer.EndArray();
        return buffer.GetString();
    }

};

}
}
