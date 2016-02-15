
#pragma once

#include "predef.hpp"
#include <ctime>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/random/mersenne_twister.hpp>
#include "stage/random.hpp"
#include "Aside.hpp"
#include "Signer.hpp"

namespace taboo {

typedef std::size_t identy_t;
typedef boost::shared_ptr<Value> SharedValue;

class SessionManager;

// Access Token
class Session
{
    friend class SessionManager;
protected:
    SharedValue _filters;
    identy_t identy;
    std::time_t expire;

public:
    Session():
        _filters(new Value), identy(0), expire(0)
    {}

    Session(identy_t _identy, Value& __filters, std::time_t expireTill):
        _filters(new Value), identy(_identy), expire(expireTill)
    {
        _filters->Swap(__filters);
    }

    const SharedValue& filters() const
    {
        return _filters;
    }

    operator bool() const
    {
        return !!identy;
    }
};

typedef boost::unordered_map<std::string, Session> SessionMap;
typedef boost::unordered_map<identy_t, std::string> TokenMap;

class SessionManager
{
private:
    static SessionManager* _instance;

    typedef stage::RandNumberGenerator<boost::mt19937_64, uint64_t> RandGenerator;
    static RandGenerator rander;

    const Session absentSession;

protected:
    SessionMap sessions;
    TokenMap tokens;

    mutable boost::shared_mutex mutex;

public:
    const std::string& ensure(identy_t identy, Value& filters, std::time_t expireTill)
    {
        {
            ReadLock lock(mutex);
            TokenMap::iterator tokenIt = tokens.find(identy);
            if (tokenIt != tokens.end()) {
                return tokenIt->second;
                SessionMap::const_iterator sessionIt = sessions.find(tokenIt->second);
                if (CS_BLIKELY(sessionIt != sessions.end())) {
                    return sessionIt->first;;
                }   // else is impossible.
            }
        }

        {
            Session sess(identy, filters, expireTill);
            WriteLock lock(mutex);
            std::string token = createToken();
            tokens.insert(std::make_pair(identy, token));
            return sessions.insert(std::make_pair(token, sess)).first->first;
        }
    }

    const Session& auth(const std::string& token) const
    {
        ReadLock lock(mutex);
        SessionMap::const_iterator it = sessions.find(token);
        if (it != sessions.end()) {
            if (std::time(NULL) < it->second.expire) {
                return it->second;
            }
        }
        return absentSession;
    }

    static SessionManager* instance()
    {
        return _instance;
    }

protected:
    std::string createToken() const
    {
        std::string token = _createToken();
        while (CS_BUNLIKELY(sessions.find(token) != sessions.end())) {
            token = _createToken();
        }
        return token;
    }

    std::string _createToken() const
    {
        timeval time;
        gettimeofday(&time, NULL);
        uint64_t rand = rander();
        MD5Stream stream;
        if (!Config::instance()->manageKey.empty()) {
            stream << Config::instance()->manageKey;
        }
        stream << StrRef(reinterpret_cast<char*>(&time.tv_sec), sizeof(time.tv_sec))
            << StrRef(reinterpret_cast<char*>(&time.tv_usec), sizeof(time.tv_usec))
            << StrRef(reinterpret_cast<char*>(&rand), sizeof(rand));
        if (!Config::instance()->manageSecret.empty()) {
            stream << Config::instance()->manageSecret;
        }
        return stream.hex();
    }

    std::string __createToken() const
    {
        std::string token;
        token.reserve(32);
        timeval time;
        gettimeofday(&time, NULL);
        hex(token, static_cast<uint32_t>(time.tv_sec));
        hex(token, static_cast<uint32_t>(time.tv_usec));
        hex(token, rander());
        return token;
    }

    template<typename IntType>
    void hex(std::string& out, IntType val) const
    {
        const char* data = reinterpret_cast<char*>(&val);
        for (const char* end = data + sizeof(val); data != end; ++data) {
            hexChr(out, *data);
        }
    }

    void hexChr(std::string& out, char chr) const
    {
        hexHalfChr(out, chr & 0xf);
        hexHalfChr(out, (chr >> 4) & 0xf);
    }

    void hexHalfChr(std::string& out, char halfChr) const
    {
        out += static_cast<char>((halfChr > 9) ? (halfChr - 9 + 'a') : (halfChr + '0'));
    }
};

}
