
#pragma once

#include "predef.hpp"
#include <string>
#include "stage/math.hpp"
#include "Aside.hpp"

namespace taboo
{

class Query
{
private:
    Dom body;

    std::string prefix;
    Value* filters;
    Value* excludes;
    ValueSet fields;

public:
    Query():
        filters(NULL), excludes(NULL)
    {}

public:
    static bool fromStr(Query& query, const char* str)
    {
        query.body.Clear();
        query.body.Parse(str);
        if (query.body.HasParseError()) {
            return false;
        }

        query.prefix.clear();
        {
            Dom::ConstMemberIterator it = query.body.FindMember(Aside::instance()->keyPrefix);
            if (it == query.body.MemberEnd() || !prefixValid(it->value)) {
                return false;
            }
            query.prefix = it->value.GetString();
        }

        query.filters = NULL;
        {
            Dom::ConstMemberIterator it = query.body.FindMember(Aside::instance()->keyFilters);
            if (it != query.body.MemberEnd()) {
                if (!it->value.IsObject()) {
                    return false;
                }
                if (!it->value.ObjectEmpty()) {
                    query.filters = &it->value;
                }
            }
        }

        query.excludes = NULL;
        {
            Dom::ConstMemberIterator it = query.body.FindMember(Aside::instance()->keyExcludes);
            if (it != query.body.MemberEnd()) {
                if (it->value.IsObject()) {
                    if (!it->value.ObjectEmpty()) {
                        query.excludes = &it->value;
                    }
                } else if (it->value.IsArray()) {
                    if (!it->value.Empty()) {
                        query.excludes = &it->value;
                    }
                } else if (it->value.IsUint()) {
                    query.excludes = &it->value;
                } else if (!it->value.IsNull()) {
                    return false;
                }
            }
        }

        fields.clear();
        {
            Dom::ConstMemberIterator it = query.body.FindMember(Aside::instance()->keyFields);
            if (it != query.body.MemberEnd()) {
                if (it->value.IsArray()) {
                    for (Value::ConstMemberIterator i = it->value.MemberBegin(); i != it->value.MemberEnd(); ++i) {
                        if (!i->value.IsString()) {
                            return false;
                        }
                        fields.insert(&i->value);
                    }
                } else if (it->value.IsString()) {
                    fields.insert(&it->value);
                }
            }
        }

        return true;
    }

    static bool prefixValid(const Value& _prefix)
    {
        return stage::between<std::size_t>(_prefix.GetStringLength(), Config::instance()->prefixMinLen, Config::instance()->prefixMaxLen);
    }
};

}
