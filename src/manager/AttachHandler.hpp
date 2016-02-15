
#pragma once

#include "BaseHandler.hpp"
#include "../Keeper.hpp"

namespace taboo {
namespace manager {

class AttachHandler:
    public BaseHandler,
    public taboo::HandlerCreator<AttachHandler>,
    private ManagerECAlloctor<2>
{
    using ManagerECAlloctor<2>::ECA;
protected:
    enum {
        err_no_keys         = ECA::ECC<1>::value,
        err_create_item     = ECA::ECC<2>::value,
        err_already_exists  = ECA::ECC<3>::value,
        err_attach_keys     = ECA::ECC<4>::value,
        err_attach_item     = ECA::ECC<5>::value,
    };

    static const uint32_t maxParamNum = 10;

    static const SharedReply okReply;

protected:
    virtual SharedResult deal() const
    {
        SharedResult res(new Result);
        do {
            KeyList keys = getKeys();
            if (keys.empty()) {
                res->code = err_no_keys;
                break;
            }

            ParamMap::const_iterator itItem = params.find(config->keyMUItem);
            if (itItem == params.end()) {
                res->code = err_bad_param;
                break;
            }
            SharedItem item = taboo::makeItem(itItem->second.c_str());
            if (!item) {
                res->code = err_create_item;
                break;
            }

            ParamMap::const_iterator itUpsert = params.find(config->keyMUUpsert);
            bool upsert = itUpsert == params.end() || itUpsert->second.empty() || itUpsert->second[0] != '0';
            if (!Keeper::instance()->attach(keys, item, upsert)) {
                res->code = err_already_exists;
                break;
            }
            res->code = err_ok;
            res->reply = okReply;

        } while (false);
        return res;
    }

    virtual ec_t checkParams() const
    {
        return (!(config->checkSign && sign.empty())
           && params.find(config->keyMUKey) != params.end()
           && params.find(config->keyMUPrefixes) != params.end()
           && params.find(config->keyMUItem) != params.end()) ?
               err_ok : err_bad_param;
    }

    KeyList getKeys() const
    {
        KeyList keys;
        ParamMap::const_iterator it = params.find(config->keyMUPrefixes);
        if (it != params.end()) {
            Dom prefixes;
            prefixes.Parse(it->second.c_str());
            if (prefixes.IsArray()) {
                for (Dom::ValueIterator i = prefixes.Begin(); i != prefixes.End(); ++i) {
                    keys.push_back(std::string(i->GetString(), i->GetStringLength()));
                }
            } else if (prefixes.IsArray()) {
                keys.push_back(std::string(prefixes.GetString(), prefixes.GetStringLength()));
            }
        }
        return keys;
    }

public:
    static void initReplys()
    {
        const_cast<SharedReply&>(okReply) = genReply(err_ok, "", mem_mode_persist);
        fillReply(err_no_keys, "no prefixes supplied");
        fillReply(err_create_item, "failed on creating item");
        fillReply(err_already_exists, "all keys already exist or item already exists");
        fillReply(err_attach_keys, "failed on attaching prefixes");
        fillReply(err_attach_item, "failed on attaching item");
    }
};

}
}
