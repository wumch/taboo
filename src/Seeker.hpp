
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

        bool operator()(id_t funnelId) const
        {
            return Config::instance()->maxIterations < ++iterated ? false : pump(funnelId);
        }

    private:
        bool pump(id_t funnelId) const
        {
            const Funnel& funnel = seeker->farm.funnel(funnelId);
            CS_DUMP(funnel.size());
            if (!funnel.empty()) {
                for (Funnel::const_iterator it = funnel.begin(); it != funnel.end(); ++it) {
                    CS_DUMP(it->second);
                    if (seeker->items.size() < maxMatch) {
                        if (recorded.find(it->second) == recorded.end()) {
                            CS_SAY("not recorded");
                            const SharedItem& item = seeker->farm.item(it->second);
                            CS_DUMP(!!item);
                            if (item && seeker->filter.apply(item)) {
                                CS_DUMP(item->id);
                                recorded.insert(item->id);
                                seeker->items.push_back(item);
                            }
                        }
                    }
                }
            }
            CS_DUMP(seeker->items.size());
            return seeker->items.size() < maxMatch;
        }
    };
};

}
