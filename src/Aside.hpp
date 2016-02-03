
#pragma once

#include "predef.hpp"
#include <boost/pool/object_pool.hpp>
#include "Config.hpp"
#include "Farm.hpp"
#include "predef.hpp"
#include "Item.hpp"
#include "Trie.hpp"

extern int main(int, char*[]);

namespace taboo
{

class Aside
{
private:
    typedef boost::object_pool<Item> ItemPool;

public:
    ItemPool itemPool;

    const Value keyId;

    SlotMap slots;

    Farm farm;

    Trie trie;

    static Aside* instance()
    {
        return _instance;
    }

private:
    static Aside* _instance;

    Aside():
        itemPool(Config::instance()->itemsAllocStep, Config::instance()->maxItems),
        keyId(Config::instance()->idKey.data(), Config::instance()->idKey.length()),
        farm(slots)
    {}

    friend int ::main(int, char*[]);
    static void initialize()
    {
        _instance = new Aside;
    }
};

}
