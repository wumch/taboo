
#pragma once

#include "../predef.hpp"
#include "BaseHandler.hpp"

namespace taboo {
namespace manager {

class TokenHandler:
    public BaseHandler,
    public HandlerCreator<TokenHandler>,
    private ManagerECAlloctor<3>
{
    using ManagerECAlloctor<2>::ECA;
protected:
    enum {

    };
    virtual SharedResult deal() const
    {
        SharedResult res(new Result);

    }
};

}
}

