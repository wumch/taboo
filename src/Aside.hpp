
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

class ValueHasher
{
public:
    int operator()(const Value& value) const
    {
        if (value.IsInt()) {
            return boost::hash<int64_t>()(value.GetInt64());
        } else if (value.IsString()) {
            return stage::SDBMHash()(value.GetString(), value.GetStringLength());
        } else if (value.IsNull()) {
            return boost::hash<uint8_t>()(0);
        } else if (value.IsBool()) {
            return boost::hash<bool>()(value.GetBool());;
        } else if (value.IsDouble()) {
            return boost::hash<double>()(value.GetDouble());;
        }
        return boost::hash<uint8_t>()(0);
    }
};

typedef boost::unordered_set<Value, ValueHasher> ValueSet;

class Aside
{
public:
    const Value keyManageKey, keyPrefixes, keyItem, keyId,
        keyPrefix, keyFilters, keyExcludes, keyFields, keyNum,
        keyErrCode, keyErrDesc;

    ValueSet queryVisibleFields, queryInvisibleFields;
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
        keyManageKey(Config::instance()->keyManageKey.data(), Config::instance()->keyManageKey.length()),
        keyPrefixes(Config::instance()->keyPrefixes.data(), Config::instance()->keyPrefixes.length()),
        keyItem(Config::instance()->keyItem.data(), Config::instance()->keyItem.length()),
        keyId(Config::instance()->keyId.data(), Config::instance()->keyId.length()),
        keyPrefix(Config::instance()->keyPrefix.data(), Config::instance()->keyPrefix.length()),
        keyFilters(Config::instance()->keyFilters.data(), Config::instance()->keyFilters.length()),
        keyExcludes(Config::instance()->keyExcludes.data(), Config::instance()->keyExcludes.length()),
        keyFields(Config::instance()->keyFields.data(), Config::instance()->keyFields.length()),
        keyNum(Config::instance()->keyNum.data(), Config::instance()->keyNum.length()),
        keyErrCode(Config::instance()->keyErrCode.data(), Config::instance()->keyErrCode.length()),
        keyErrDesc(Config::instance()->keyErrDesc.data(), Config::instance()->keyErrDesc.length()),
        farm(slots)
    {}

    friend int ::main(int, char*[]);
    static bool initialize()
    {
        _instance = new Aside;
        const Config* config = Config::instance();
        for (StringList::const_iterator it = config->queryVisibleFields.begin(); it != config->queryVisibleFields.end(); ++it) {
            Value field(it->data(), it->length());
            _instance->queryVisibleFields.insert(field.Move());
        }
        return _instance->_initialize();
    }

    bool _initialize();
};

}
