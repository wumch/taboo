
#pragma once

#include "predef.hpp"
#include <list>
typedef std::list<Item> ItemList;
#if __cplusplus < 201103L
#   include <boost/unordered_map.hpp>
    typedef boost::unordered_map<id_t, ItemList> ItemMap;
#else
#   include <unordered_map>
    typedef std::unordered_map<id_t, ItemList> ItemMap;
#endif
#include "item.hpp"

namespace taboo
{

class Farm
{
private:
    ItemMap map;
    const ItemList empty_items;
    const Item dummy_item;

public:
    void attach(const Item& item)
    {
        ItemMap::iterator it = map.find(item.id);
        if (it == map.end())
        {
            it = map.insert(std::make_pair(item.id, empty_items));
        }
        ItemList::iterator pos = std::find(it->second.begin(), it->second.end(), item);
        if (pos == it->second.end())
        {
            it->second.push_back(item);
        }
    }

    bool detach(const Item& item)
    {
        ItemMap::iterator it = map.find(item.id);
        if (it != map.end())
        {
            ItemList::iterator pos = std::find(it->second.begin(), it->second.end(), item);
            if (pos != it->second.end())
            {
                it->second.erase(pos);
                return true;
            }
        }
        return false;
    }

    const ItemList& items(id_t id) const
    {
        ItemMap::iterator it = map.find(item.id);
        return it == map.end() ? empty_items : it->second;
    }

    const Item& item(id_t id) const
    {
        ItemMap::iterator it = map.find(item.id);
        if (it != map.end())
        {
            for (ItemList::iterator i = it->second.begin(); i != it->second.end(); ++i)
            {
                if (i->id == id)
                {
                    return i;
                }
            }
        }
        return item;
    }
};

}
