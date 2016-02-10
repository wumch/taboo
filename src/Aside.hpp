
#pragma once

#include "predef.hpp"
#include <boost/pool/object_pool.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lock_types.hpp>
#include "Config.hpp"
#include "Farm.hpp"
#include "predef.hpp"
#include "Item.hpp"
#include "Trie.hpp"

extern int main(int, char*[]);

namespace taboo
{

typedef boost::shared_lock<boost::shared_mutex> ReadLock;
typedef boost::unique_lock<boost::shared_mutex> WriteLock;

class Aside
{
private:
    typedef boost::object_pool<Item> ItemPool;

public:
    ItemPool itemPool;

    const Value keyId, keyManageKey, keyPrefixes, keyItem,
    keyPrefix, keyFilters, keyExcludes, keyFields, keyNum,
        keyErrCode, keyErrDesc;

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
        itemPool(Config::instance()->itemsAllocStep, Config::instance()->maxItems),
        keyItem(Config::instance()->keyItem.data(), Config::instance()->keyItem.length()),
        keyManageKey(Config::instance()->keyManageKey.data(), Config::instance()->keyManageKey.length()),
        keyPrefixes(Config::instance()->keyPrefixes.data(), Config::instance()->keyPrefixes.length()),
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
        return _instance->_initialize();
    }

    bool _initialize();
};

}
