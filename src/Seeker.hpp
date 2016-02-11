
#pragma once

#include "predef.hpp"
#include <algorithm>
#include <boost/unordered_set.hpp>
#include "Aside.hpp"
#include "Config.hpp"
#include "Farm.hpp"
#include "Item.hpp"
#include "Trie.hpp"
#include "Filter.hpp"
#include "Query.hpp"

namespace taboo {

class Seeker
{
private:
    const Trie& trie;

    const Farm& farm;

    mutable Query query;

    mutable FilterChain filter;

    mutable ItemPtrList items;

public:
    Seeker():
        trie(Aside::instance()->trie),
        farm(Aside::instance()->farm)
    {}

    const ItemPtrList& seek(const char* _query) const
    {
        items.clear();
        if (Query::rebuild(query, _query) &&
            FilterChain::rebuild(filter, query)) {
            seek();
        }
        return items;
    }

private:
    void seek() const
    {
        ItemCallback cb(this, query.num);
        ReadLock lock(Aside::instance()->accessMutex);
        trie.traverse(query.prefix, cb);
    }

    class ItemCallback
    {
    private:
        typedef boost::unordered_set<id_t> ItemIdSet;
        const Seeker* const seeker;
        ItemIdSet recorded;
        const std::size_t maxMatch;
        std::size_t iterated;

    public:
        ItemCallback(const Seeker* _seeker, std::size_t _maxMatch):
            seeker(_seeker), maxMatch(_maxMatch), iterated(0)
        {}

        bool operator()(id_t slotId)
        {
            return Config::instance()->maxIterations < ++iterated ? false : pump(slotId);
        }

    private:
        bool pump(id_t slotId)
        {
            const Slot& slot = seeker->farm.slot(slotId);
            if (!slot.empty()) {
                for (Slot::const_iterator it = slot.begin(); it != slot.end(); ++it) {
                    if (seeker->items.size() < maxMatch) {
                        CS_DUMP(seeker->items.size());
                        if (recorded.find(it->second->id) == recorded.end()) {
                            if (seeker->filter.apply(it->second)) {
                                recorded.insert(it->second->id);
                                seeker->items.push_back(it->second);
                            }
                        }
                    }
                }
            }
            return seeker->items.size() < maxMatch;
        }
    };
};

}
