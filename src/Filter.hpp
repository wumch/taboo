
#pragma once

#include "predef.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_set.hpp>
#include "rapidjson/document.h"
#include "Item.hpp"

namespace taboo
{

class BaseFilter
{
protected:
    const Value& attr;
    const BaseFilter* next;

public:
    explicit BaseFilter(const Value& _attr):
        attr(_attr), next(NULL)
    {}

    virtual bool apply(const ItemPtr& item) const
    {
        return _apply(item) ? (has_next() ? next->apply(item) : true) : false;
    }

    void attach(const BaseFilter* _next)
    {
        next = _next;
    }

    static bool validate(const Value& _attr)
    {
        return true;
    }

    virtual ~BaseFilter()
    {
        delete next;
    }

protected:
    virtual bool _apply(const ItemPtr& item) const = 0;

    bool has_next() const
    {
        return next;
    }
};

typedef boost::shared_ptr<BaseFilter> FilterPtr;


class EmptyFilter:
    public BaseFilter
{
public:
    EmptyFilter():
        BaseFilter(Value())
    {}

    virtual bool _apply(const ItemPtr& item) const
    {
        return true;
    }

    static EmptyFilter* create()
    {
        return new EmptyFilter;
    }
};

class EqualFilter:
    public BaseFilter
{
protected:
    const Value& value;

public:
    EqualFilter(const Value& _attr, const Value& _value):
        BaseFilter(_attr), value(_value)
    {}

    virtual bool _apply(const ItemPtr& item) const
    {
        rapidjson::Document::ConstMemberIterator it = item->doc.FindMember(attr);
        return (it == item->doc.MemberEnd()) ? false : (it->value == value);
    }

    static boost::shared_ptr<EqualFilter> create(const Value& _attr, const Value& _value)
    {
        return boost::make_shared<EqualFilter>(_attr, _value);
    }
};

class UnequalFilter:
    public BaseFilter
{
protected:
    const Value& value;

public:
    UnequalFilter(const Value& _attr, const Value& _value):
        BaseFilter(_attr), value(_value)
    {}

    virtual bool _apply(const ItemPtr& item) const
    {
        rapidjson::Document::ConstMemberIterator it = item->doc.FindMember(attr);
        return (it == item->doc.MemberEnd()) ? true : (it->value != value);
    }

    static boost::shared_ptr<UnequalFilter> create(const Value& _attr, const Value& _value)
    {
        return boost::make_shared<UnequalFilter>(_attr, _value);
    }
};

class ValueHasher
{
public:
    int operator()(const Value& value) const
    {
        return boost::hash<std::string>()(value.GetString());
    }
};

class InFilter:
    public BaseFilter
{
protected:
    typedef boost::unordered_set<Value, ValueHasher> ValueSet;
    ValueSet values;

public:
    explicit InFilter(const Value& _attr, const Value& _values):
        BaseFilter(_attr)
    {
        fill_values(_values);
    }

    virtual bool _apply(const ItemPtr& item) const
    {
        Value::MemberIterator it = item->doc.FindMember(attr);
        if (it != item->doc.MemberEnd()) {
            return values.find(it->value) != values.end();
        }
        return false;
    }

    static boost::shared_ptr<InFilter> create(const Value& _attr, const Value& _values)
    {
        return boost::make_shared<InFilter>(_attr, _values);
    }

    static bool validate(const Value& _attr, const Value& _value)
    {
        return _value.IsArray();
    }

protected:
    void fill_values(const Value& _values)
    {
        for (rapidjson::Document::ConstMemberIterator it = _values.MemberBegin();
            it != _values.MemberEnd(); ++it) {
//            values.insert(it->value);
        }
    }
};

class RangeFilter:
    public BaseFilter
{
protected:
    const Value& min, &max;

public:
    RangeFilter(const Value& _attr, const Value& _min, const Value& _max):
        BaseFilter(_attr), min(_min), max(_max)
    {}

    virtual bool _apply(const ItemPtr& item) const
    {
        Value::MemberIterator it = item->doc.FindMember(attr);
        if (it != item->doc.MemberEnd()) {
//            if (!min.IsNull() && it->value < min) {
//                return false;
//            }
//            if (!max.IsNull() && max < it->value) {
//                return false;
//            }
            return true;
        }
        return false;
    }

    static boost::shared_ptr<RangeFilter> create(const Value& _attr, const Value& _min, const Value& _max)
    {
        return boost::make_shared<RangeFilter>(_attr, _min, _max);
    }
};

class AttrFilter:
    public BaseFilter
{
protected:
    const Value& value;

public:
    explicit AttrFilter(const Value& _attr, const Value& _value):
        BaseFilter(_attr), value(_value)
    {}

    virtual bool _apply(const ItemPtr& item) const
    {
        Value::MemberIterator it = item->doc.FindMember(attr);
        if (it != item->doc.MemberEnd()) {
            // todo: implement
        }
        return false;
    }
};


// Filter responsibility chain
class FilterChain
{
protected:
    typedef boost::shared_ptr<BaseFilter> Filter;
    Filter match, exclude;
    static const Value key_in, key_range, key_attr, key_exclude;

public:
    static Filter build(const rapidjson::Document& cond)
    {
        Filter filter(EmptyFilter::create());
        for (rapidjson::Document::ConstMemberIterator it = cond.MemberBegin();
            it != cond.MemberEnd(); ++it) {
            if (it->value.IsArray()) {
                if (InFilter::validate(it->name, it->value)) {
                    filter->attach(InFilter::create(it->name, it->value).get());
                }
            } else {

            }
        }
        return filter;
    }
};

}
