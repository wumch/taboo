
#pragma once

#include "predef.hpp"
#include "item.hpp"

namespace taboo
{

class Farm
{
private:
    ItemMap& items;
    const ItemList empty_items;
    const Item dummy_item;

public:
    explicit Farm(ItemMap& _items):
        items(_items)
    {}

    void attach(const Item& item)
    {
        ItemMap::iterator it = items.find(item.id);
        if (it == items.end())
        {
            it = items.insert(std::make_pair(item.id, empty_items));
        }
        ItemList::iterator pos = std::find(it->second.begin(), it->second.end(), item);
        if (pos == it->second.end())
        {
            it->second.push_back(item);
        }
    }

    bool detach(const Item& item)
    {
        ItemMap::iterator it = items.find(item.id);
        if (it != items.end())
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

    const ItemList& get_items(id_t id) const
    {
        ItemMap::iterator it = items.find(item.id);
        return it == items.end() ? empty_items : it->second;
    }

    const Item& item(id_t id) const
    {
        ItemMap::iterator it = items.find(item.id);
        if (it != items.end())
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
