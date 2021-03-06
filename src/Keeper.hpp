
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

    static Keeper* _instance;

public:
    static Keeper* instance()
    {
        return _instance;
    }

    static bool initialize()
    {
        _instance = new Keeper;
        return true;
    }

    bool attach(const KeyList& keys, const SharedItem& item, bool upsertItem)
    {
        AttachCallback cb(farm, item, upsertItem);
        WriteLock lock(Aside::instance()->accessMutex);
        return (trie.attach(keys, cb) && cb.attached) || upsertItem;
    }

    bool update_item(const std::string& key, const SharedItem& item)
    {
        id_t id;
        {
            ReadLock lock(Aside::instance()->accessMutex);
            id = trie[key];
        }
        if (id) {
            WriteLock lock(Aside::instance()->accessMutex);
            return farm.attach(id, item);
        }
        return false;
    }

    // TODO: 重前缀 会重复出现
    bool detach(const KeyList& keys, const SharedItem& item)
    {
        EraseCallback cb(farm, item);
        WriteLock lock(Aside::instance()->accessMutex);
        return trie.erase(keys, item->id, cb);
    }

private:
    Keeper():
        trie(Aside::instance()->trie),
        farm(Aside::instance()->farm)
    {}

    class AttachCallback
    {
    private:
        Farm& farm;
        const SharedItem& item;
        const bool upsert;

    public:
        mutable bool attached;

        AttachCallback(Farm& _farm, const SharedItem& _item, bool _upsert):
            farm(_farm), item(_item), upsert(_upsert), attached(false)
        {}

        void operator()(id_t id) const
        {
            attached = farm.attach(id, item, upsert);
        }
    };

    class EraseCallback
    {
    private:
        Farm& farm;
        const SharedItem& item;

    public:
        EraseCallback(Farm& _farm, const SharedItem& _item):
            farm(_farm), item(_item)
        {}

        void operator()(id_t funnelId) const
        {
            farm.attach(funnelId, item);
        }
    };
};

}
