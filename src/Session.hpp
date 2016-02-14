
#pragma once

#include "predef.hpp"
#include <ctime>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include "Aside.hpp"

namespace taboo {

typedef std::size_t identy_t;
typedef boost::shared_ptr<Value> SharedValue;

// Access Token
class Session
{
public:
    SharedValue filters;
    identy_t identy;
    std::time_t expire;

    Session(identy_t _identy, Value& _filters, std::time_t expireTill):
        filters(new Value), identy(_identy), expire(expireTill)
    {
        filters->Swap(_filters);
    }

    operator bool() const
    {
        return !!identy;
    }
};

typedef boost::unordered_map<std::string, Session> SessionMap;
typedef boost::unordered_map<identy_t, std::string> TokenMap;

class TokenManager
{
protected:
    SessionMap sessions;
    TokenMap tokens;

    Session absentSession;

    boost::shared_mutex mutex;

public:
    const std::string& ensure(Value& data);

    const Session& auth(const std::string& token) const
    {
        SessionMap::const_iterator it = sessions.find(token);
        return (it == sessions.end()) ? absentSession : it->second;
    }

protected:
    const std::string& ensure(identy_t identy, Value& filters, std::time_t expireTill)
    {
        {
            ReadLock lock(mutex);
            TokenMap::iterator tokenIt = tokens.find(identy);
            if (tokenIt != tokens.end()) {
                return tokenIt->second;
                SessionMap::const_iterator sessionIt = sessions.find(tokenIt->second);
                if (sessionIt != sessions.end()) {
                    return sessionIt->first;;
                } else {
                    tokens.erase(tokenIt);
                }
            }
        }

        {
            std::string token = createToken();
            Session sess(identy, filters, expireTill);
            WriteLock lock(mutex);
            tokens.insert(std::make_pair(identy, token));
            return sessions.insert(std::make_pair(token, sess)).first->first;
        }
    }
    
    std::string createToken() const;
};

}
