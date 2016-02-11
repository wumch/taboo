
#pragma once

#include "BaseHandler.hpp"

namespace taboo {
namespace manager {

class StatusHandler:
    public BaseHandler, public taboo::HandlerCreator<StatusHandler>
{
public:
    virtual SharedResult deal() const
    {
        SharedResult res;
        return res;
    }

    static void initReplys() {}
};

}
}
