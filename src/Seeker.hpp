
#pragma once

#include "predef.hpp"
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <boost/unordered_set.hpp>
#include "Aside.hpp"
#include "Config.hpp"
#include "Farm.hpp"
#include "Item.hpp"
#include "Trie.hpp"

namespace taboo
{

class Seeker
{
private:
    const Trie& trie;

    const Farm& farm;

    const std::size_t max_num;

    mutable ItemPtrSet items;

public:
    Seeker():
        trie(Aside::instance()->trie),
        farm(Aside::instance()->farm),
        max_num(Config::instance()->max_matches)
    {
        items.reserve(max_num);
    }

    const ItemPtrSet& seek(const std::string& key, std::size_t num) const
    {
        match(key, std::min(max_num, num));
        return items;
    }

    void match(const std::string& key, std::size_t num) const
    {
        items.clear();
        CS_RETURN_IF(key.empty() || max_num < 1);
        ItemCallback cb(farm, items, num);
        trie.traverse(key, cb);
    }

private:
    class ItemCallback
    {
    private:
        const Farm& _farm;
        ItemPtrSet& _items;
        const std::size_t _max_num;

    public:
        ItemCallback(const Farm& __farm, ItemPtrSet& __items, std::size_t __max_num):
            _farm(__farm), _items(__items), _max_num(__max_num)
        {}

        bool operator()(id_t id)
        {
            return push_id(id);
        }

    private:
        bool push_id(id_t id)
        {
            const Slot& slot = _farm.slot(id);
            if (!slot.empty()) {
                for (Slot::const_iterator it = slot.begin(); it != slot.end(); ++it) {
                    if (_items.size() < _max_num) {
                        _items.insert(it->second);
                    }
                }
            }
            return _items.size() < _max_num;
        }
    };
};

}
