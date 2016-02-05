
#pragma once

#include "predef.hpp"
#include <string>
#include <list>
#define USE_FAST_LOAD
#include "cedar/cedar.h"
#include "Item.hpp"

namespace taboo
{

typedef std::list<std::string> KeyList;

class Trie
{
private:
    enum {
        no_value = static_cast<id_t>(-1),
        no_path  = static_cast<id_t>(-2),
    };
    typedef cedar::da<id_t,no_value, no_path, false> DA;

    DA da;

public:
    template<typename Callback>
    bool attach(const KeyList& keys, id_t id, Callback& cb)
    {
        bool attached = false;
        for (KeyList::const_iterator it = keys.begin(); it != keys.end(); ++it)
        {
            std::size_t node_pos = 0, key_pos = 0;
            id_t slotId = da.traverse(it->data(), node_pos, key_pos, it->length());
            if (slotId == no_value || slotId == no_path) {
                slotId = da.update(it->data(), it->length(), id);
                attached = true;
            }
            cb(slotId);
        }
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
        std::size_t node_pos = 0, key_pos = 0;
        id_t id = da.traverse(key.data(), node_pos, key_pos, key.length());
        if (id != no_path) {
            if (id != no_value) {
                if (!cb(id)) {
                    return;
                }
            }
            std::size_t root = node_pos;
            id = da.begin(node_pos, key_pos);
            while (id != no_path) {
                if (!cb(id)) {
                    return;
                }
                id = da.next(node_pos, key_pos, root);
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
