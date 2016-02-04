
#include "Item.hpp"
#include "rapidjson/error/en.h"
#include "Config.hpp"
#include "Aside.hpp"

namespace taboo
{

ItemPtr makeItem(const char* str)
{
//    ItemPtr item(Aside::instance()->itemPool.malloc());
    ItemPtr item(new Item);
    item->doc.Parse(str);
    if (CS_BLIKELY(!item->doc.HasParseError())) {
        Value::MemberIterator it = item->doc.FindMember(Aside::instance()->keyId);
        if (CS_BLIKELY(it != item->doc.MemberEnd())) {
            if (CS_BLIKELY(it->value.IsUint())) {
                item->id = it->value.GetUint();
            } else {
                LOG_EVERY_N(ERROR, 10) << "value of 'item'.'id' must be of type uint32_t." " (" << google::COUNTER << ")";
            }
        } else {
            LOG_EVERY_N(ERROR, 10) << "'item' must have an 'id'." " (" << google::COUNTER << ")";
        }
    } else {
        LOG(ERROR) << "error occured while parsing 'item': " << rapidjson::GetParseError_En(item->doc.GetParseError());
    }
    return item;
}

}
