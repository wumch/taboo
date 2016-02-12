
#pragma once

#include "../predef.hpp"
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
    friend class Router;
    using QueryECAlloctor<0>::ECA;
protected:
    enum {
        err_bad_request_method      = ECA::ECC<1>::value,
        err_bad_request             = ECA::ECC<2>::value,
        err_too_many_params         = ECA::ECC<3>::value,
        err_bad_param               = ECA::ECC<4>::value,
    };

    Query query;

public:
    BasePredicter() {}

    std::string predict() const;

    virtual ~BasePredicter();

protected:
    std::string formReply(const SharedItemList& items) const
    {
        return query.fields.empty() ? formReplyWhole(items) : formReplyFields(items);
    }

    const char* formReplyFields(const SharedItemList& items) const
    {
        rapidjson::StringBuffer buffer(0, Config::instance()->querySendBuffer);
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        writer.StartArray();
        for (SharedItemList::const_iterator item = items.begin(); item != items.end(); ++item) {
            const Dom& dom = (*item)->dom;
            writer.StartObject();
            for (ValueSet::const_iterator key = query.fields.begin(); key != query.fields.end(); ++key) {
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

    const char* formReplyWhole(const SharedItemList& items) const
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
