
#pragma once

#include "BaseHandler.hpp"

namespace taboo {
namespace manager {

class StatusHandler:
    public BaseHandler, public taboo::HandlerCreator<StatusHandler>
{
public:
    virtual ResPtr deal() const
    {
        ResPtr res;
        return res;
    }
};

}
}
