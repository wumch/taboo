
#pragma once

#include "predef.hpp"
#include <boost/pool/object_pool.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/unordered_set.hpp>
#include "stage/hash.hpp"
#include "Config.hpp"
#include "Farm.hpp"
#include "predef.hpp"
#include "Item.hpp"
#include "Trie.hpp"

extern int main(int, char*[]);

namespace taboo {

class Portal;

typedef boost::shared_lock<boost::shared_mutex> ReadLock;
typedef boost::unique_lock<boost::shared_mutex> WriteLock;

class ValuePtrHasher
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
        return boost::hash<uint8_t>()(0);
    }
};

class ValuePtrEqualer
{
public:
    bool operator()(const Value* lhs, const Value* rhs) const
    {
        return *lhs == *rhs;
    }
};

typedef boost::unordered_set<const Value*, ValuePtrHasher, ValuePtrEqualer> ValuePtrSet;

class Aside
{
private:
    const Config* config;

public:
    const std::string keyMUKey, keyMUSign, keyMUPrefixes, keyMUItem, keyMUUpsert;
    const Value keyMDErrCode, keyMDErrDesc, keyMDPayload, keyMDToken, keyMDTokenExpire,
        keyId, keyQEchoData,
        keyQUPayload, keyQUToken, keyQUPrefix, keyQUFilters, keyQUExcludes, keyQUFields, keyQUNum,
        keyQDErrCode, keyQDErrDesc, keyQDPayload;

    ValuePtrSet queryVisibleFields, queryInvisibleFields;
    bool queryVisibleAll;

    SlotMap slots;

    Farm farm;

    Trie trie;

    boost::shared_mutex accessMutex;;

    static Aside* instance()
    {
        return _instance;
    }

private:
    static Aside* _instance;

    Aside():
        config(Config::instance()),
        keyMUKey(config->keyMUKey),
        keyMUSign(config->keyMUSign),
        keyMUPrefixes(config->keyMUPrefixes),
        keyMUItem(config->keyMUItem),
        keyMUUpsert(config->keyMUUpsert),

        keyMDErrCode(config->keyMDErrCode.data(), config->keyMDErrCode.length()),
        keyMDErrDesc(config->keyMDErrDesc.data(), config->keyMDErrDesc.length()),
        keyMDPayload(config->keyMDPayload.data(), config->keyMDPayload.length()),
        keyMDToken(config->keyMDToken.data(), config->keyMDToken.length()),
        keyMDTokenExpire(config->keyMDTokenExpire.data(), config->keyMDTokenExpire.length()),

        keyId(config->keyId.data(), config->keyId.length()),
        keyQEchoData(config->keyQEchoData.data(), config->keyQEchoData.length()),

        keyQUPayload(config->keyQUPayload.data(), config->keyQUPayload.length()),
        keyQUToken(config->keyQUToken.data(), config->keyQUToken.length()),
        keyQUPrefix(config->keyQUPrefix.data(), config->keyQUPrefix.length()),
        keyQUFilters(config->keyQUFilters.data(), config->keyQUFilters.length()),
        keyQUExcludes(config->keyQUExcludes.data(), config->keyQUExcludes.length()),
        keyQUFields(config->keyQUFields.data(), config->keyQUFields.length()),
        keyQUNum(config->keyQUNum.data(), config->keyQUNum.length()),

        keyQDErrCode(config->keyQDErrCode.data(), config->keyQDErrCode.length()),
        keyQDErrDesc(config->keyQDErrDesc.data(), config->keyQDErrDesc.length()),
        keyQDPayload(config->keyQDPayload.data(), config->keyQDPayload.length()),

        queryVisibleAll(config->queryVisibleAll),
        farm(slots)
    {
        if (!queryVisibleAll) {
            if (!config->queryVisibleFields.empty() &&
                !(config->queryVisibleFields.size() == 1 && *config->queryVisibleFields.begin() == "*")) {
                for (StringList::const_iterator it = config->queryVisibleFields.begin();
                    it != config->queryVisibleFields.end(); ++it) {
                    queryVisibleFields.insert(new Value(it->data(), it->length()));
                }
            }
            if (!config->queryInvisibleFields.empty()) {
                for (StringList::const_iterator it = config->queryInvisibleFields.begin();
                    it != config->queryInvisibleFields.end(); ++it) {
                    queryInvisibleFields.insert(new Value(it->data(), it->length()));
                }
            }
        }
    }

    friend class Portal;
    static bool initialize()
    {
        _instance = new Aside;
        return _instance->_initialize();
    }

    bool _initialize();
};

}
