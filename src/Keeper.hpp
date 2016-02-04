
#pragma once

#include "predef.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "Aside.hpp"
#include "Trie.hpp"
#include "Farm.hpp"
#include "Item.hpp"

namespace taboo
{

class Keeper
{
private:
    Trie& trie;

    Farm& farm;

public:
    Keeper():
        trie(Aside::instance()->trie),
        farm(Aside::instance()->farm)
    {}

    bool attach(const KeyList& keys, const ItemPtr& item)
    {
        AttachCallback cb(farm, item);
        return trie.attach(keys, item->id, cb);
    }

    bool update_item(const std::string& key, const ItemPtr& item)
    {
        id_t id = trie[key];
        if (id) {
            return farm.detach(id, item);
        }
        return false;
    }

    // TODO: 重前缀 会重复出现
    bool detach(const KeyList& keys, const ItemPtr& item)
    {
        EraseCallback cb(farm, item);
        return trie.erase(keys, item->id, cb);
    }

private:
    class AttachCallback
    {
    private:
        Farm& farm;
        const ItemPtr& item;

    public:
        AttachCallback(Farm& _farm, const ItemPtr& _item):
            farm(_farm),
            item(_item)
        {}

        void operator()(id_t id) const
        {
            farm.attach(id, item);
        }
    };

    class EraseCallback
    {
    private:
        Farm& farm;
        const ItemPtr& item;

    public:
        EraseCallback(Farm& _farm, const ItemPtr& _item):
            farm(_farm), item(_item)
        {}

        void operator()(id_t id) const
        {
            farm.attach(id, item);
        }
    };
};

}
