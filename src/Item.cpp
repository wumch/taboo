
#include "Item.hpp"
#include "rapidjson/error/en.h"
#include "Config.hpp"
#include "Aside.hpp"

namespace taboo
{

SharedItem makeItem(const char* str)
{
    SharedItem item(new Item), res;
    item->dom.Parse(str);
    if (CS_BLIKELY(!item->dom.HasParseError())) {
        Value::MemberIterator it = item->dom.FindMember(Aside::instance()->keyId);
        if (CS_BLIKELY(it != item->dom.MemberEnd())) {
            if (CS_BLIKELY(it->value.IsUint())) {
                item->id = it->value.GetUint();
                res = item;
            }
        }
    }
    return res;
}

}
