
#pragma once

#include "predef.hpp"
#include <algorithm>
#include <string>
#include <boost/unordered_set.hpp>
#include "rapidjson/error/en.h"
#include "stage/hash.hpp"
#include "stage/math.hpp"
#include "Aside.hpp"

namespace taboo {

class Query
{
private:
    Dom body;

public:
    ValuePtrSet fields;
    std::string prefix;
    Value* filters, * excludes;
    std::size_t num;
    bool fieldsAll;

public:
    Query():
        filters(NULL), excludes(NULL), num(0)
    {}

public:
    static bool rebuild(Query& query, const std::string& str)
    {
        if (!query.body.IsNull()) {
            query.body.SetNull();
        }
        query.body.Parse(str.c_str());
        if (query.body.HasParseError() || !query.body.IsObject()) {
            query.body.SetNull();
            LOG_EVERY_N(ERROR, 10) << "failed on build query: "
                << rapidjson::GetParseError_En(query.body.GetParseError())
                << ", JSON: " << str;
            CS_DUMP("bad json for query");
            return false;
        }

        query.prefix.clear();
        {
            Dom::ConstMemberIterator it = query.body.FindMember(Aside::instance()->keyPrefix);
            if (it == query.body.MemberEnd() || !prefixValid(it->value)) {
                CS_SAY("no key-prefix");
                return false;
            }
            query.prefix = it->value.GetString();
            if (query.prefix.empty()) {
                CS_SAY("empy prefix");
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
                    CS_SAY("bad filters");
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
                    CS_SAY("bad excludes");
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
                            CS_SAY("bad fields");
                            return false;
                        }
                        query.fields.insert(&i->value);
                    }
                } else if (it->value.IsString()) {
                    query.fields.insert(&it->value);
                }
            }
            reviseFields(query);
            if (!query.fieldsAll && query.fields.empty()) {
                CS_SAY("no fields");
                return false;
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

    static void reviseFields(Query& query)
    {
        const Aside* const aside = Aside::instance();
        if (aside->queryVisibleAll) {
            query.fieldsAll = query.fields.empty();
            return;
        }
        query.fieldsAll = false;
        if (aside->queryInvisibleFields.empty()) {
            if (query.fields.empty()) {
                for (ValuePtrSet::const_iterator it = aside->queryVisibleFields.begin();
                    it != aside->queryVisibleFields.end(); ++it) {
                    query.fields.insert(*it);
                }
            } else {
                for (ValuePtrSet::const_iterator it = query.fields.begin();
                    it != query.fields.end(); ++it) {
                    if (aside->queryVisibleFields.find(*it) == aside->queryVisibleFields.end()) {
                        it = query.fields.erase(it);
                    }
                }
            }
        } else {
            for (ValuePtrSet::iterator it = query.fields.begin();
                it != query.fields.end(); ++it) {
                if (aside->queryInvisibleFields.find(*it) != aside->queryInvisibleFields.end()) {
                    it = query.fields.erase(it);
                }
            }
        }
    }

    static bool prefixValid(const Value& _prefix)
    {
        return stage::between<std::size_t>(_prefix.GetStringLength(), Config::instance()->prefixMinLen, Config::instance()->prefixMaxLen);
    }
};

}
