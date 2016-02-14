
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
    Value* echoData;
    Value* filters, * excludes;
    std::size_t num;
    bool fieldsAll;

public:
    Query():
        echoData(NULL), filters(NULL), excludes(NULL), num(0)
    {}

public:
    bool rebuild(const std::string& str)
    {
        if (!body.IsNull()) {
            body.SetNull();
        }
        body.Parse(str.c_str());
        if (body.HasParseError() || !body.IsObject()) {
            body.SetNull();
            LOG_EVERY_N(ERROR, 10) << "failed on build query: "
                << rapidjson::GetParseError_En(body.GetParseError())
                << ", JSON: " << str;
            CS_DUMP("bad json for query");
            return false;
        }

        prefix.clear();
        {
            Dom::ConstMemberIterator it = body.FindMember(Aside::instance()->keyPrefix);
            if (it == body.MemberEnd() || !prefixValid(it->value)) {
                CS_SAY("no key-prefix");
                return false;
            }
            prefix = it->value.GetString();
            if (prefix.empty()) {
                CS_SAY("empy prefix");
                return false;
            }
        }

        filters = NULL;
        {
            Dom::MemberIterator it = body.FindMember(Aside::instance()->keyFilters);
            if (it != body.MemberEnd()) {
                if (it->value.IsObject()) {
                    if (!it->value.ObjectEmpty()) {
                        filters = &it->value;
                    }
                } else if (it->value.IsArray()) {
                    if (!it->value.Empty()) {
                        filters = &it->value;
                    }
                } else if (it->value.IsUint()) {
                    filters = &it->value;
                } else if (!it->value.IsNull()) {
                    CS_SAY("bad filters");
                    return false;
                }
            }
        }

        excludes = NULL;
        {
            Dom::MemberIterator it = body.FindMember(Aside::instance()->keyExcludes);
            if (it != body.MemberEnd()) {
                if (it->value.IsObject()) {
                    if (!it->value.ObjectEmpty()) {
                        excludes = &it->value;
                    }
                } else if (it->value.IsArray()) {
                    if (!it->value.Empty()) {
                        excludes = &it->value;
                    }
                } else if (it->value.IsUint()) {
                    excludes = &it->value;
                } else if (!it->value.IsNull()) {
                    CS_SAY("bad excludes");
                    return false;
                }
            }
        }

        fields.clear();
        {
            Dom::MemberIterator it = body.FindMember(Aside::instance()->keyFields);
            if (it != body.MemberEnd()) {
                if (it->value.IsArray()) {
                    for (Value::ConstValueIterator i = it->value.Begin(); i != it->value.End(); ++i) {
                        if (!i->IsString()) {
                            CS_SAY("bad fields");
                            return false;
                        }
                        fields.insert(&*i);
                    }
                } else if (it->value.IsString()) {
                    fields.insert(&it->value);
                }
            }
            reviseFields();
            if (!fieldsAll && fields.empty()) {
                CS_SAY("no fields");
                return false;
            }
        }

        num = Config::instance()->defaultMatches;
        {
            Dom::MemberIterator it = body.FindMember(Aside::instance()->keyNum);
            if (it != body.MemberEnd()) {
                if (it->value.IsUint()) {
                    num = std::min<std::size_t>(it->value.GetUint(), Config::instance()->maxMatches);
                }
            }
            if (num < 1) {
                num = Config::instance()->defaultMatches;
            }
        }

        echoData = NULL;
        {
            if (!Config::instance()->keyEchoData.empty()) {
                Dom::MemberIterator it = body.FindMember(Aside::instance()->keyEchoData);
                if (it != body.MemberEnd()) {
                    echoData = &it->value;
                }
            }
        }

        return true;
    }

    void reviseFields()
    {
        const Aside* const aside = Aside::instance();
        if (aside->queryVisibleAll) {
            fieldsAll = fields.empty();
            return;
        }
        fieldsAll = false;
        if (aside->queryInvisibleFields.empty()) {
            if (fields.empty()) {
                for (ValuePtrSet::const_iterator it = aside->queryVisibleFields.begin();
                    it != aside->queryVisibleFields.end(); ++it) {
                    fields.insert(*it);
                }
            } else {
                for (ValuePtrSet::const_iterator it = fields.begin();
                    it != fields.end(); ++it) {
                    if (aside->queryVisibleFields.find(*it) == aside->queryVisibleFields.end()) {
                        it = fields.erase(it);
                    }
                }
            }
        } else {
            for (ValuePtrSet::iterator it = fields.begin();
                it != fields.end(); ++it) {
                if (aside->queryInvisibleFields.find(*it) != aside->queryInvisibleFields.end()) {
                    it = fields.erase(it);
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
