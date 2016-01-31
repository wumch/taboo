
#pragma once

#include "Farm.hpp"
#include "predef.hpp"
#include "Item.hpp"
#include "Trie.hpp"

namespace taboo
{

class Aside
{
public:
    SlotMap slots;

    Farm farm;

    Trie trie;

    static Aside* instance()
    {
        return &_instance;
    }

private:
    static Aside _instance;

    Aside():
        farm(slots)
    {}
};

}
