
#pragma once

#include "BaseHandler.hpp"

namespace taboo {
namespace manager {

class DetachHandler:
    public BaseHandler, public taboo::HandlerCreator<DetachHandler>
{
protected:
    enum {
        err_detach = 203001,
        err_detach_keys = 203002,
        err_detach_item = 203003,
    };
public:
    virtual ResPtr deal() const
    {
        ResPtr res(new Res(err_detach));
        return res;
    }
};

}
}
