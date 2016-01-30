
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

class Keeper
{
private:
    Trie& trie;

    Farm& farm;

public:
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

};

}
