
#pragma once

#include "predef.hpp"
#include <algorithm>
#include <vector>
#include <string>
#include <list>
#include <boost/unordered_map.hpp>
#include "attr.hpp"

namespace taboo
{

typedef std::vector<Attr> AttrList;

class Item
{
public:
    AttrList attrs;
    const id_t id;

    Item(id_t _id):
        id(_id)
    {}

    Item():
        id(0)
    {}

    bool operator!() const
    {
        return id;
    }
};

inline bool operator==(const Item& lhs, const Item& rhs)
{
    return lhs.id == rhs.id;
}

#include <unordered_map>
typedef boost::unordered_map<id_t, Item, int> ItemSlot;
typedef boost::unordered_map<id_t, ItemSlot, int> ItemSlotMap;

}
