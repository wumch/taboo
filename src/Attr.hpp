
#pragma once

#include <string>
#include "rapidjson/document.h"

namespace taboo
{

class Attr
{
public:
    std::string name;
    void* value;
};

}
