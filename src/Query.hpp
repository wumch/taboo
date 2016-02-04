
#pragma once

#include "predef.hpp"
#include <algorithm>
#include <string>
#include <boost/unordered_set.hpp>
#include "rapidjson/error/en.h"
#include "stage/hash.hpp"
#include "stage/math.hpp"
#include "Aside.hpp"

namespace taboo
{

class ValueHasher
{
public:
    int operator()(const Value* value) const
    {
        if (value->IsInt()) {
            return boost::hash<int64_t>()(value->GetInt64());
        } else if (value->IsString()) {
            return stage::SDBMHash()(value->GetString(), value->GetStringLength());
        } else if (value->IsNull()) {
            return boost::hash<uint8_t>()(0);
        } else if (value->IsBool()) {
            return boost::hash<bool>()(value->GetBool());;
        } else if (value->IsDouble()) {
            return boost::hash<double>()(value->GetDouble());;
        }
        LOG_EVERY_N(WARNING, 100) << "hash unroll mismatched";
        return boost::hash<uint8_t>()(0);
    }
};

class ValueEqualer
{
public:
    bool operator()(const Value* lhs, const Value* rhs) const
    {
        return *lhs == *rhs;
    }
};

typedef boost::unordered_set<const Value*, ValueHasher, ValueEqualer> ValueSet;


class Query
{
private:
    Dom body;

public:
    ValueSet fields;
    std::string prefix;
    Value* filters, * excludes;
    std::size_t num;

public:
    Query():
        filters(NULL), excludes(NULL), num(0)
    {}

public:
    static bool rebuild(Query& query, const char* str)
    {
        if (!query.body.IsNull()) {
            query.body.SetNull();
        }
        query.body.Parse(str);
        if (query.body.HasParseError() || !query.body.IsObject()) {
            query.body.SetNull();
            LOG_EVERY_N(ERROR, 10) << "failed on build query: "
                << rapidjson::GetParseError_En(query.body.GetParseError())
                << ", JSON: " << str;
            return false;
        }

        query.prefix.clear();
        {
            Dom::ConstMemberIterator it = query.body.FindMember(Aside::instance()->keyPrefix);
            if (it == query.body.MemberEnd() || !prefixValid(it->value)) {
                return false;
            }
            query.prefix = it->value.GetString();
            if (query.prefix.empty()) {
                return false;
            }
        }

        query.filters = NULL;
        {
            Dom::MemberIterator it = query.body.FindMember(Aside::instance()->keyFilters);
            if (it != query.body.MemberEnd()) {
                if (it->value.IsObject()) {
                    if (!it->value.ObjectEmpty()) {
                        query.filters = &it->value;
                    }
                } else if (it->value.IsArray()) {
                    if (!it->value.Empty()) {
                        query.filters = &it->value;
                    }
                } else if (it->value.IsUint()) {
                    query.filters = &it->value;
                } else if (!it->value.IsNull()) {
                    return false;
                }
            }
        }

        query.excludes = NULL;
        {
            Dom::MemberIterator it = query.body.FindMember(Aside::instance()->keyExcludes);
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

        query.fields.clear();
        {
            Dom::MemberIterator it = query.body.FindMember(Aside::instance()->keyFields);
            if (it != query.body.MemberEnd()) {
                if (it->value.IsArray()) {
                    for (Value::ConstMemberIterator i = it->value.MemberBegin(); i != it->value.MemberEnd(); ++i) {
                        if (!i->value.IsString()) {
                            return false;
                        }
                        query.fields.insert(&i->value);
                    }
                } else if (it->value.IsString()) {
                    query.fields.insert(&it->value);
                }
            }
        }

        query.num = Config::instance()->defaultMatches;
        {
            Dom::MemberIterator it = query.body.FindMember(Aside::instance()->keyNum);
            if (it != query.body.MemberEnd()) {
                if (it->value.IsUint()) {
                    query.num = std::min<std::size_t>(it->value.GetUint(), Config::instance()->maxMatches);
                }
            }
            if (query.num < 1) {
                query.num = Config::instance()->defaultMatches;
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
