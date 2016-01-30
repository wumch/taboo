
#pragma once

#include "predef.hpp"
#include <boost/unordered_map.hpp>
#include "item.hpp"

namespace taboo
{

class Farm
{
private:
    ItemSlotMap& slots;
    const ItemSlot empty_slot;
    const Item dummy_item;

public:
    explicit Farm(ItemSlotMap& item_slots):
        slots(item_slots)
    {}

    void attach(const Item& item)
    {
        ItemSlotMap::iterator it = slots.find(item.id);
        if (it == slots.end())
        {
            it = slots.insert(std::make_pair(item.id, empty_slot));
        }
        ItemSlot::iterator pos = std::find(it->second.begin(), it->second.end(), item);
        if (pos == it->second.end())
        {
            it->second.insert(std::make_pair(item.id, item));
        }
    }

    bool detach(const Item& item)
    {
        ItemSlotMap::iterator it = slots.find(item.id);
        if (it != slots.end())
        {
            ItemSlot::iterator pos = std::find(it->second.begin(), it->second.end(), item);
            if (pos != it->second.end())
            {
                it->second.erase(pos);
                if (it->second.empty())
                {
                    slots.erase(it);
                }
                return true;
            }
        }
        return false;
    }

    const ItemSlot& slot(id_t id) const
    {
        // todo: 加读锁
        ItemSlotMap::iterator it = slots.find(id);
        return it == slots.end() ? empty_slot : it->second;
    }

    const Item& item(id_t id) const
    {
        ItemSlotMap::iterator it = slots.find(id);
        if (it != slots.end())
        {
            ItemSlot::const_iterator pos = it->second.find(id);
            if (pos != it->second.end())
            {
                return *pos;
            }
        }
        return item;
    }
};

}
