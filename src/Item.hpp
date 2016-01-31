
#pragma once

#include "predef.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include "rapidjson/document.h"

namespace taboo
{

class Item
{
public:
    typedef rapidjson::Document Doc;
    Doc doc;
    id_t id;

    explicit Item(id_t _id):
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

typedef boost::shared_ptr<Item> ItemPtr;
inline bool operator==(const ItemPtr& lhs, const ItemPtr& rhs)
{
    return lhs->id == rhs->id;
}

class ItemPtrHasher
{
public:
    int operator()(const ItemPtr& item) const
    {
        return boost::hash<int>()(item->id);
    }
};

inline ItemPtr make_item(const char* str)
{
    ItemPtr item(new Item);
    item->doc.Parse(str);
    if (CS_BLIKELY(!item->doc.HasParseError() || item->doc.HasMember("id"))) {
        item->id = item->doc["id"].GetUint();
    }
    return item;
}

typedef boost::unordered_set<ItemPtr, ItemPtrHasher> ItemPtrSet;
typedef boost::unordered_map<id_t, ItemPtr> Slot;
typedef boost::unordered_map<id_t, Slot> SlotMap;

}
