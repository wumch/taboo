
#pragma once

#include "predef.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <glog/logging.h>
#include "rapidjson/document.h"
#include "stage/misc.hpp"

namespace taboo {

typedef uint32_t id_t;
typedef rapidjson::Document Dom;
typedef rapidjson::Value Value;

class Item
{
public:
    Dom dom;
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

typedef boost::shared_ptr<Item> SharedItem;

extern SharedItem makeItem(const char* str);

inline bool operator==(const Item& lhs, const Item& rhs)
{
    return lhs.id == rhs.id;
}

inline bool operator==(const SharedItem& lhs, const SharedItem& rhs)
{
    return lhs->id == rhs->id;
}

class SharedItemHasher
{
public:
    int operator()(const SharedItem& item) const
    {
        return boost::hash<id_t>()(item->id);
    }
};

typedef std::vector<SharedItem> SharedItemList;
typedef boost::unordered_map<id_t, SharedItem> Slot;
typedef boost::unordered_map<id_t, Slot> SlotMap;

}
