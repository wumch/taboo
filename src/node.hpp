
#pragma once

#include "predef.hpp"
#include <algorithm>
#include <list>
#include "item.hpp"

namespace taboo
{

class Node
{
private:
    typedef std::list<Node*> Children;
    typedef std::list<id_t> IdList;
    Children children;
    IdList items;
    uint64_t item_num:56;
    uint64_t value:8;

public:
    Node(nval_t val):
        item_num(0), value(val)
    {}

    bool contains(nval_t val) const
    {
        static Node* tmp = create(0);   // todo: mess
        tmp->value = val;
        return std::binary_search(children.begin(), children.end(), tmp, Cmper());
    }

    Node* find_child(nval_t val) const
    {
        static Node* tmp = create(0);   // todo: mess
        tmp->value = val;
        Children::const_iterator pos = std::lower_bound(children.begin(), children.end(), tmp, Cmper());
        if (pos == children.end() || (*pos)->value != tmp->value)
        {
            return NULL;
        }
        return *pos;
    }

    Node* attach_child(nval_t val)
    {
        static Node* tmp = create(0);   // todo: mess
        tmp->value = val;
        Children::iterator pos = std::lower_bound(children.begin(), children.end(), tmp, Cmper());
        if (pos == children.end() || (*pos)->value != tmp->value)
        {
            pos = children.insert(pos, create(val));
        }
        return *pos;
    }

    void attach_item(id_t id)
    {
        IdList::iterator pos = std::lower_bound(items.begin(), items.end(), id);
        if (pos == items.end() || *pos != id)
        {
            pos = items.insert(pos, id);
            ++item_num;
        }
    }

    IdList getItems() const
    {
        return items;
    }

    static Node* create(nval_t val)
    {
        return new Node(val);   // todo: memory pool
    }

private:
    class Cmper
    {
    public:
        bool operator()(const Node* left, const Node* right) const
        {
            return left->value < right->value;
        }
    };
};

}