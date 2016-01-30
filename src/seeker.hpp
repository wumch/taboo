
#pragma once

#include "predef.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "item.hpp"
#include "farm.hpp"
#include "aside.hpp"

namespace taboo
{

typedef std::vector<std::string> KeyList;
typedef std::vector<id_t> ItemIdList;
typedef std::vector<const Item*> ItemPtrList;

class Seeker
{
private:
    const Trie& trie;

    const Farm& farm;

public:
    Seeker():
        trie(Aside::instance()->trie),
        farm(Aside::instance()->farm)
    {}

    void match(const std::string& key, ItemIdList& res, ItemPtrList& items)
    {
        match(key, res, items, items.capacity());
    }

    void match(const std::string& key, ItemIdList& item_ids, ItemPtrList& item_ptrs, std::size_t max_num)
    {
        CS_RETURN_IF(max_num < 1);
        retrieve_ids(key, item_ids, max_num);
        CS_RETURN_IF(item_ids.empty());

        for (ItemIdList::iterator it = item_ids.begin(); it != item_ids.end(); ++it)
        {
            const ItemList& items = farm.items(*it);
            if (!items.empty())
            {
                for (ItemList::const_iterator i = items.begin(); i != items.end(); ++i)
                {
                    item_ptrs.push_back(&*i);
                    if (item_ptrs.size() >= max_num)
                    {
                        break;
                    }
                }
            }
        }
    }

    void retrieve_ids(const std::string& key, ItemIdList& res)
    {
        return retrieve_ids(key, res, res.capacity());
    }

    void retrieve_ids(const std::string& key, ItemIdList& res, std::size_t max_num)
    {
        res.resize(std::min(trie.commonPrefixPredict(key.data(), res.data(), max_num, key.length()), res.capacity()));
    }
};

}
