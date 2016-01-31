
#pragma once

#include "stage/meta.hpp"
#include <string>
#include <list>

namespace taboo
{

typedef uint32_t id_t;

enum {
    no_value = static_cast<uint32_t>(-1),
    no_path  = static_cast<uint32_t>(-2),
};

typedef std::list<std::string> KeyList;

}
