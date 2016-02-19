
#pragma once

#include "predef.hpp"
#include <string>
#include <list>
#define USE_FAST_LOAD
#   include "cedar/cedar.h"
#undef USE_FAST_LOAD
#include "Item.hpp"

namespace taboo
{

typedef std::list<std::string> KeyList;

class Trie
{
private:
    enum {
//        no_value = static_cast<id_t>(-1),
//        no_path  = static_cast<id_t>(-2),
        no_value = 0x7fffffff - 1,
        no_path  = 0x7fffffff - 2,
    };
    typedef cedar::da<id_t,no_value, no_path, false> DA;

    DA da;

public:
    template<typename Callback>
    bool attach(const KeyList& keys, id_t id, Callback& cb)
    {
        bool attached = false;
        for (KeyList::const_iterator it = keys.begin(); it != keys.end(); ++it) {
            std::size_t nodePos = 0, keyPos = 0;
            id_t funnelId = da.traverse(it->data(), nodePos, keyPos, it->length());
            if (funnelId == no_value || funnelId == no_path) {
                funnelId = da.update(it->data(), it->length(), id);
                attached = true;
            }
            cb(funnelId);
        }
        CS_DUMP(da.size());
        return attached;
    }

    // 解除 keys => item 的关联，但不删除 item
    bool detach(const KeyList& keys, id_t id)
    {
        return false;
    }

    // 解除 keys => item 的关联，并删除 item
    template<typename Callback>
    bool erase(const KeyList& keys, id_t id, Callback& cb)
    {
        // 找到位置
        // 找出新的 value
        // 更新value  或者直接删除
        return false;
    }

    template<typename Callback>
    void traverse(const std::string& key, Callback& cb) const
    {
        std::size_t nodePos = 0, keyPos = 0;
        id_t funnelId = da.traverse(key.data(), nodePos, keyPos, key.length());
        if (funnelId != no_path) {
            if (funnelId != no_value) {
                if (!cb(funnelId)) {
                    return;
                }
            }
            std::size_t root = nodePos;
            funnelId = da.begin(nodePos, keyPos);
            while (funnelId != no_path) {
                if (!cb(funnelId)) {
                    return;
                }
                funnelId = da.next(nodePos, keyPos, root);
            }
        }
    }

    id_t operator[](const std::string& key) const
    {
        int64_t id = da.exactMatchSearch<id_t>(key.data(), key.length());
        return (id == no_value) ? 0 : id;
    }
};

}
