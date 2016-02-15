
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
    const Value keyManageKey, keyPrefixes, keyItem, keyId, keyToken,
        keyQueryToken, keyPrefix, keyFilters, keyExcludes, keyFields, keyNum,
        keyEchoData, keyErrCode, keyErrDesc, keyPayload;

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
        keyManageKey(config->keyMUKey.data(), config->keyMUKey.length()),
        keyPrefixes(config->keyMUPrefixes.data(), config->keyMUPrefixes.length()),
        keyItem(config->keyMUItem.data(), config->keyMUItem.length()),
        keyId(config->keyId.data(), config->keyId.length()),
        keyPrefix(config->keyQUPrefix.data(), config->keyQUPrefix.length()),
        keyQueryToken(config->keyQUToken.data(), config->keyQUToken.length()),
        keyFilters(config->keyQUFilters.data(), config->keyQUFilters.length()),
        keyExcludes(config->keyQUExcludes.data(), config->keyQUExcludes.length()),
        keyFields(config->keyQUFields.data(), config->keyQUFields.length()),
        keyNum(config->keyQUNum.data(), config->keyQUNum.length()),
        keyEchoData(config->keyQEchoData.data(), config->keyQEchoData.length()),
        keyErrCode(config->keyMDErrCode.data(), config->keyMDErrCode.length()),
        keyErrDesc(config->keyMDErrDesc.data(), config->keyMDErrDesc.length()),
        keyPayload(config->keyQDPayload.data(), config->keyQDPayload.length()),
        queryVisibleAll(config->queryVisibleAll), farm(slots)
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

    friend int ::main(int, char*[]);
    static bool initialize()
    {
        _instance = new Aside;
        return _instance->_initialize();
    }

    bool _initialize();
};

}
