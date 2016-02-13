
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

    mutable FilterChain filter;

    mutable SharedItemList items;

public:
    Seeker():
        trie(Aside::instance()->trie),
        farm(Aside::instance()->farm)
    {}

    const SharedItemList& seek(const Query& query) const
    {
        items.clear();
        FilterChain::rebuild(filter, query);
        _seek(query);
        return items;
    }

private:
    void _seek(const Query& query) const
    {
        ItemCallback cb(this, query.num);
        ReadLock lock(Aside::instance()->accessMutex);
        trie.traverse(query.prefix, cb);
    }

    class ItemCallback
    {
    private:
        typedef boost::unordered_set<id_t> ItemIdSet;
        mutable ItemIdSet recorded;
        const Seeker* const seeker;
        const std::size_t maxMatch;
        mutable std::size_t iterated;

    public:
        ItemCallback(const Seeker* _seeker, std::size_t _maxMatch):
            seeker(_seeker), maxMatch(_maxMatch), iterated(0)
        {}

        bool operator()(id_t slotId) const
        {
            return Config::instance()->maxIterations < ++iterated ? false : pump(slotId);
        }

    private:
        bool pump(id_t slotId) const
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
