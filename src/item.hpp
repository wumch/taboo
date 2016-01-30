
#pragma once

#include "predef.hpp"
#include <vector>
#include <string>
#include <list>
#include <algorithm>
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
        return !!id;
    }
};

inline bool operator==(const Item& lhs, const Item& rhs)
{
    return lhs.id == rhs.id;
}

typedef std::list<Item> ItemList;
#if __cplusplus < 201103L
#   include <boost/unordered_map.hpp>
    typedef boost::unordered_map<id_t, ItemList> ItemMap;
#else
#   include <unordered_map>
    typedef std::unordered_map<id_t, ItemList> ItemMap;
#endif

}