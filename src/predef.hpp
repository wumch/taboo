
#pragma once

#include "stage/meta.hpp"
#include <string>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <glog/logging.h>
#include "rapidjson/document.h"

namespace taboo
{

typedef uint32_t id_t;

enum {
    no_value = static_cast<uint32_t>(-1),
    no_path  = static_cast<uint32_t>(-2),
};

typedef std::list<std::string> KeyList;

typedef rapidjson::Document Dom;
typedef boost::shared_ptr<Dom> DomPtr;
typedef rapidjson::Value Value;
typedef boost::shared_ptr<Value> ValPtr;


class ValueHasher
{
public:
    int operator()(const Value* value) const
    {
        return boost::hash<const char*>()(value->GetString());
    }
};

typedef boost::unordered_set<const Value*, ValueHasher> ValueSet;

}
