
#pragma once

#include "predef.hpp"
#include <algorithm>
#include "Aside.hpp"
#include "Config.hpp"
#include "Farm.hpp"
#include "Item.hpp"
#include "Trie.hpp"
#include "Filter.hpp"
#include "Query.hpp"

namespace taboo
{

class Seeker
{
private:
    const Trie& trie;

    const Farm& farm;

    mutable Query query;

    mutable ItemPtrSet items;

public:
    Seeker():
        trie(Aside::instance()->trie),
        farm(Aside::instance()->farm)
    {}

    const ItemPtrSet& seek(const char* _query) const
    {
        Query::fromStr(query, _query);
        match(query);
        return items;
    }

    void match(const Query& query) const
    {
        items.clear();
        FilterChain filter = FilterChain::build(query);
        ItemCallback cb(farm, items, filter, query.num);
        trie.traverse(query.prefix, cb);
    }

private:
    class ItemCallback
    {
    private:
        const Farm& _farm;
        ItemPtrSet& _items;
        FilterChain& filter;
        const std::size_t _max_num;
        std::size_t iterations;

    public:
        ItemCallback(const Farm& __farm, ItemPtrSet& __items, FilterChain& _filter, std::size_t __max_num):
            _farm(__farm), _items(__items), filter(_filter), _max_num(__max_num), iterations(0)
        {}

        bool operator()(id_t id)
        {
            return CS_BUNLIKELY(Config::instance()->maxIterations < ++iterations) ? false : push_one(id);
        }

    private:
        bool push_one(id_t id)
        {
            const Slot& slot = _farm.slot(id);
            if (!slot.empty()) {
                for (Slot::const_iterator it = slot.begin(); it != slot.end(); ++it) {
                    if (_items.size() < _max_num) {
                        CS_DUMP(_items.size());
                        if (filter.apply(it->second)) {
                            _items.insert(it->second);
                        }
                    }
                }
            }
            return _items.size() < _max_num;
        }
    };
};

}
