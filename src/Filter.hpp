
#pragma once

#include "predef.hpp"
#include <cstring>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/functional/hash.hpp>
#include "rapidjson/document.h"
#include "Item.hpp"
#include "Query.hpp"

namespace taboo
{

class BaseFilter;
typedef boost::shared_ptr<BaseFilter> FilterPtr;

class BaseFilter
{
protected:
    const Value& attr;
    FilterPtr next;

public:
    explicit BaseFilter(const Value& _attr):
        attr(_attr)
    {}

    virtual bool apply(const ItemPtr& item) const
    {
        return _apply(item) ? (hasNext() ? next->apply(item) : true): false;
    }

    void attach(const FilterPtr& _next)
    {
        next = _next;
    }

    static bool validate(const Value& _value)
    {
        return false;
    }

    FilterPtr& getNext()
    {
        return next;
    }

    virtual ~BaseFilter()
    {
        CS_SAY("destructing " << (uint64_t)this);
    }

protected:
    virtual bool _apply(const ItemPtr& item) const = 0;

    bool hasNext() const
    {
        return next;
    }
};


template<bool applyRes>
class EmptyFilterImpl:
    public BaseFilter
{
public:
    EmptyFilterImpl():
        BaseFilter(Value())
    {}

    static boost::shared_ptr<EmptyFilterImpl> create()
    {
        return boost::make_shared<EmptyFilterImpl>();
    }

protected:
    virtual bool _apply(const ItemPtr& item) const
    {
        return applyRes;
    }
};

typedef EmptyFilterImpl<true> EmptyFilter;
typedef EmptyFilterImpl<false> EmptyExcludeFilter;

template<bool resOnEqual>
class EqualFilterImpl:
    public BaseFilter
{
protected:
    const Value& value;
    bool mustMiss;

public:
    EqualFilterImpl(const Value& _attr, const Value& _value):
        BaseFilter(_attr), value(_value), mustMiss(value.IsNull())
    {
        CS_SAY("equal filter built");
    }

    static boost::shared_ptr<EqualFilterImpl> create(const Value& _attr, const Value& _value)
    {
        return boost::make_shared<EqualFilterImpl>(_attr, _value);
    }

    static bool validate(const Value& _value)
    {
        return !_value.IsObject() && !_value.IsArray() && !_value.IsNull();
    }

protected:
    virtual bool _apply(const ItemPtr& item) const
    {
        rapidjson::Document::ConstMemberIterator it = item->dom.FindMember(attr);
        if (mustMiss) {
            return (it == item->dom.MemberEnd()) == resOnEqual;
        } else {
            return (it != item->dom.MemberEnd() && it->value == value) == resOnEqual;
        }
    }
};

typedef EqualFilterImpl<true> EqualFilter;
typedef EqualFilterImpl<false> UnequalFilter;

class InFilter:
    public BaseFilter
{
protected:
    ValueSet values;
    bool allowMiss;

public:
    explicit InFilter(const Value& _attr, const Value& _values):
        BaseFilter(_attr), allowMiss(false)
    {
        fill_values(_values);
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
    virtual bool _apply(const ItemPtr& item) const
    {
        Value::MemberIterator it = item->dom.FindMember(attr);
        if (it != item->dom.MemberEnd()) {
            return values.find(&it->value) != values.end();
        }
        return allowMiss;
    }

    void fill_values(const Value& _values)
    {
        for (rapidjson::Document::ConstValueIterator it = _values.Begin();
            it != _values.End(); ++it) {
            if (it->IsNull()) {
                allowMiss = true;
            } else {
                values.insert(&*it);
            }
        }
    }
};

// todo: currently too much crude.
inline bool operator<(const Value& lhs, const Value& rhs)
{
    if (lhs.GetType() == rhs.GetType()) {
        if (lhs.IsInt()) {
            return lhs.GetInt() < rhs.GetInt();
        } else if (lhs.IsDouble()) {
            return lhs.GetDouble() < rhs.GetDouble();
        } else if (lhs.IsString()) {
            return std::strcmp(lhs.GetString(), rhs.GetString()) < 0;
        }
    }
    return false;
}

class RangeFilter:
    public BaseFilter
{
protected:
    const Value& min, & max;

public:
    RangeFilter(const Value& _attr, const Value& _min, const Value& _max):
        BaseFilter(_attr), min(_min), max(_max)
    {}

    static bool validate(const Value& _value)
    {
        return _value.IsObject();   // todo: strict check
    }

    static boost::shared_ptr<RangeFilter> create(const Value& _attr, const Value& _min, const Value& _max)
    {
        return boost::make_shared<RangeFilter>(_attr, _min, _max);
    }

protected:
    virtual bool _apply(const ItemPtr& item) const
    {
        Value::MemberIterator it = item->dom.FindMember(attr);
        if (it != item->dom.MemberEnd()) {
            if (!min.IsNull() && it->value < min) {
                return false;
            }
            if (!max.IsNull() && max < it->value) {
                return false;
            }
            return true;
        }
        return false;
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

    static bool validate(const Value& _value)
    {
        return _value.IsObject() && !_value.ObjectEmpty();
    }

    static boost::shared_ptr<AttrFilter> create(const Value& _attr, const Value& _value)
    {
        return boost::make_shared<AttrFilter>(_attr, _value);
    }

protected:
    virtual bool _apply(const ItemPtr& item) const
    {
        Value::MemberIterator it = item->dom.FindMember(attr);
        if (it != item->dom.MemberEnd()) {
            // todo: implement
        }
        return false;
    }
};

// Filter responsibility chain
class FilterChain
{
protected:
    FilterPtr filter;
    FilterPtr exclude;

public:
    bool apply(const ItemPtr& item) const
    {
        return (!exclude || !exclude->apply(item)) &&
            (!filter || filter->apply(item));
    }

    static bool rebuild(FilterChain& chain, const Query& query)
    {
        rebuild(chain.filter, query.filters);
        rebuild(chain.exclude, query.excludes);
        return true;
    }

private:
    static void rebuild(FilterPtr& _filter, const Value* cond)
    {
        if (cond == NULL) {
            _filter.reset();
        } else if (cond->IsArray()) {
            _filter = InFilter::create(Aside::instance()->keyId, *cond);
        } else if (cond->IsUint()) {
            _filter = EqualFilter::create(Aside::instance()->keyId, *cond);
        } else if (cond->IsObject()) {
            byCond(_filter, cond);
        }
    }

    // todo: too crude
    static void byCond(FilterPtr& _filter, const Value* cond)
    {
        BaseFilter* cursor = _filter.get();
        for (rapidjson::Document::ConstMemberIterator it = cond->MemberBegin();
            it != cond->MemberEnd(); ++it) {
            if (it->value.IsArray()) {
                if (InFilter::validate(it->name, it->value)) {
                    attach(_filter, cursor, InFilter::create(it->name, it->value));
                }
            } else if (it->value.IsObject()) {
                CS_DUMP("attr filter is not implemented");
            } else {
                if (EqualFilter::validate(it->value)) {
                    attach(_filter, cursor, EqualFilter::create(it->name, it->value));
                }
            }
        }
    }

    static void attach(FilterPtr& _filter, BaseFilter*& cursor, const FilterPtr& next)
    {
        if (!_filter) {
            _filter = next;
            cursor = _filter.get();
        } else {
            cursor->attach(next);
            cursor = cursor->getNext().get();
        }
    }
};

}
