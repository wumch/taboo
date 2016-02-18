
#pragma once

#include "BaseHandler.hpp"

namespace taboo {
namespace manager {

class StatusHandler:
    public BaseHandler, public taboo::HandlerCreator<StatusHandler>
{
    friend class taboo::Router;
public:
    virtual SharedResult deal() const
    {
        SharedResult res;
        return res;
    }

protected:
    static void initReplys() {}
};

}
}
