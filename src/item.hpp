
#pragma once

#include "predef.hpp"
#include <vector>
#include <string>
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
};

}