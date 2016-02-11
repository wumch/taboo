
#pragma once

#include "BaseHandler.hpp"
#include "../Keeper.hpp"

namespace taboo {
namespace manager {

class AttachHandler:
    public BaseHandler, public taboo::HandlerCreator<AttachHandler>
{
protected:
    enum {
        err_no_keys         = 202001,
        err_create_item     = 202002,
        err_attach          = 202003,
        err_attach_keys     = 202004,
        err_attach_item     = 202005,
    };

    static const uint32_t maxParamNum = 10;

    static const ReplyPtr okReply;

protected:
    virtual ResPtr deal() const
    {
        ResPtr res(new Res);
        do {
            KeyList keys = getKeys();
            if (keys.empty()) {
                res->code = err_no_keys;
                break;
            }

            ParamMap::const_iterator it = params.find(config->keyItem);
            if (it == params.end()) {
                res->code = err_bad_param;
                break;
            }
            ItemPtr item = taboo::makeItem(it->second.c_str());
            if (!item) {
                res->code = err_create_item;
                break;
            }

            if (!Keeper::instance()->attach(keys, item)) {
                res->code = err_attach;
                break;
            }
            res->code = err_ok;
            res->reply = okReply;

        } while (false);
        CS_DUMP(res->code);
        return res;
    }

    virtual ec_t checkParams() const
    {
        return (!(config->checkSign && sign.empty())
           && params.find(config->keyManageKey) != params.end()
           && params.find(config->keyPrefixes) != params.end()
           && params.find(config->keyItem) != params.end()) ?
               err_ok : err_bad_param;
    }

    KeyList getKeys() const
    {
        KeyList keys;
        ParamMap::const_iterator it = params.find(config->keyPrefixes);
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
        const_cast<ReplyPtr&>(okReply) = genReply(err_ok, "", mem_mode_persist);
        fillReply(err_no_keys, "no prefixes supplied");
        fillReply(err_create_item, "failed on creating item");
        fillReply(err_attach, "failed on attach ");
        fillReply(err_attach_keys, "failed on attaching prefixes");
        fillReply(err_attach_item, "failed on attaching item");
    }
};

}
}
