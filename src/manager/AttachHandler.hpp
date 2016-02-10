
#pragma once

#include "../predef.hpp"
#include "BaseHandler.hpp"

namespace taboo {
namespace manager {

class AttachHandler:
    public BaseHandler
{
protected:
    enum {
        no_prefixes = 10001,
        failed_on_create_item = 10002,
        failed_on_attach_prefixes = 10003,
        failed_on_attach_item = 10004,
    };

public:
    
};

}
}
