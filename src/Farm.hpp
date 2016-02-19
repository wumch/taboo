
#pragma once

#include "Item.hpp"
#include "predef.hpp"

namespace taboo
{

class Farm
{
private:
    ItemDict& itemDict;
    FunnelDict&funnelDict;
    const Funnel emptyFunnel;
    const SharedItem dummyItem;

public:
    explicit Farm(ItemDict& _itemDict, FunnelDict& _funnelDict):
        itemDict(_itemDict), funnelDict(_funnelDict)
    {}

    bool attach(id_t id, const SharedItem& item, bool upsert = true)
    {
        FunnelDict::iterator it = funnelDict.find(id);
        if (it == funnelDict.end()) {
             it = funnelDict.insert(std::make_pair(id, emptyFunnel)).first;
        }
        Funnel::iterator pos = it->second.find(item->id);
        bool attached = false;
        if (pos == it->second.end()) {
            it->second.insert(std::make_pair(item->id, item->id));
            attached = true;
        } else if (upsert) {
            pos->second = item->id;
            attached = true;
        }
        if (attached) {
            itemDict.insert(std::make_pair(item->id, item));
        }
        return attached;
    }

    bool detach(id_t id, const SharedItem& item)
    {
        FunnelDict::iterator it = funnelDict.find(id);
        if (it != funnelDict.end())
        {
            Funnel::iterator pos = it->second.find(item->id);
            if (pos != it->second.end())
            {
                it->second.erase(pos);
                if (it->second.empty())
                {
                    funnelDict.erase(it);
                }
                return true;
            }
            itemDict.erase(item->id);
        }
        return false;
    }

    const Funnel& funnel(id_t id) const
    {
        // todo: 加读锁
        FunnelDict::iterator it = funnelDict.find(id);
        return it == funnelDict.end() ? emptyFunnel : it->second;
    }

    const SharedItem& item(id_t id) const
    {
        FunnelDict::iterator it = funnelDict.find(id);
        if (it != funnelDict.end())
        {
            Funnel::const_iterator pos = it->second.find(id);
            if (pos != it->second.end())
            {
                ItemDict::const_iterator itItem = itemDict.find(pos->second);
                if (CS_BLIKELY(itItem != itemDict.end())) {
                    return itItem->second;
                }
            }
        }
        return dummyItem;
    }
};

}
