
#pragma once

#include "predef.hpp"
#include "cedar/cedar.h"
#include "item.hpp"
#include "farm.hpp"

namespace taboo
{

typedef cedar::da<id_t,no_value, no_path, false> Trie;

class Aside
{
public:
    ItemMap items;

    Farm farm;

    Trie trie;

    static Aside* instance()
    {
        return &_instance;
    }

private:
    static Aside _instance;

    Aside():
        farm(items)
    {}
};

}