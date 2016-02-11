
#pragma once

#include "Item.hpp"
#include "predef.hpp"

namespace taboo
{

class Farm
{
private:
    SlotMap& slots;
    const Slot empty_slot;
    const SharedItem dummy_item;

public:
    explicit Farm(SlotMap& slot_map):
        slots(slot_map)
    {}

    bool attach(id_t id, const SharedItem& item)
    {
        SlotMap::iterator it = slots.find(id);
        if (it == slots.end())
        {
             it = slots.insert(std::make_pair(id, empty_slot)).first;
        }
        Slot::iterator pos = it->second.find(item->id);
        if (pos == it->second.end())
        {
            it->second.insert(std::make_pair(item->id, item));
            return true;
        }
        return false;
    }

    bool detach(id_t id, const SharedItem& item)
    {
        SlotMap::iterator it = slots.find(id);
        if (it != slots.end())
        {
            Slot::iterator pos = it->second.find(item->id);
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

    const Slot& slot(id_t id) const
    {
        // todo: 加读锁
        SlotMap::iterator it = slots.find(id);
        return it == slots.end() ? empty_slot : it->second;
    }

    const SharedItem& item(id_t id) const
    {
        SlotMap::iterator it = slots.find(id);
        if (it != slots.end())
        {
            Slot::const_iterator pos = it->second.find(id);
            if (pos != it->second.end())
            {
                return pos->second;
            }
        }
        return dummy_item;
    }

private:
    bool insert(Slot& slot, const SharedItem& item) const
    {
        Slot::iterator pos = slot.find(item->id);
        if (pos == slot.end())
        {
            slot.insert(std::make_pair(item->id, item));
            return true;
        }
        return false;
    }
};

}
