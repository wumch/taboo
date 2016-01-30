
#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "aside.hpp"

namespace taboo
{

typedef std::vector<std::string> KeyList;
typedef std::vector<id_t> ItemIdList;
typedef std::vector<const Item*> ItemPtrList;

class Seeker
{
private:
    Trie& trie;

    Farm& farm;

public:
    Seeker():
        trie(Aside::instance()->trie),
        farm(Aside::instance()->farm)
    {}

    void attach(const KeyList& keys, const Item& item)
    {
        for (KeyList::const_iterator it = keys.begin(); it != keys.end(); ++it)
        {
            if (trie.exactMatchSearch(it->data(), it->length()) == no_value)
            {
                id_t &id = trie.update(it->data(), it->length(), 0);
                if (id == 0 || item.id < id)
                {
                    id = item.id;
                }
            }
        }
        farm.attach(item);
    }

    // TODO: 重前缀 会重复出现
    bool detach(const KeyList& keys, const Item& item)
    {
        farm.detach(item);
        if (!farm.item(item.id))    // 没有同ID的item
        {
            for (KeyList::const_iterator it = keys.begin(); it != keys.end(); ++it)
            {
                trie.erase(it->data(), it->size());
            }
        }
        return false;
    }

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