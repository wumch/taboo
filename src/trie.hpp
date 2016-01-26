
#pragma once

#include <string>
#include <vector>
#include "node.hpp"
#include "item.hpp"

namespace taboo
{

typedef std::vector<id_t> ItemIdList;

class Trie
{
private:
    Node* root;

public:
    Trie():
        root(Node::create(0))
    {}

    void attach(const std::string& key, const Item& item)
    {
        attach(reinterpret_cast<const nval_t*>(key.data()), reinterpret_cast<const nval_t*>(key.data() + key.size()), item.id);
    }

    void attach(const nval_t* key_begin, const nval_t* key_end, const uint32_t id)
    {
        Node* node = root;
        for (const nval_t* cur = key_begin; cur != key_end; ++cur)
        {
            node = node->attach_child(*cur);
        }
        node->attach_item(id);
    }

    ItemIdList match(const std::string& key) const
    {
        ItemIdList items;
        const Node* node = match(reinterpret_cast<const nval_t*>(key.data()), reinterpret_cast<const nval_t*>(key.data() + key.size()));
        CS_DUMP(node->getItems().size());
        items.push_back(*node->getItems().begin());
        return items;
    }

    const Node* match(const nval_t* begin, const nval_t* end) const
    {
        const Node* node = root;
        for (const nval_t* cur = begin; cur != end; ++cur)
        {
            node = node->find_child(*cur);
            if (node == NULL)
            {
                return NULL;
            }
        }
        return node;
    }
};

}