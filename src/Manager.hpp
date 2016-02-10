
#pragma once

#if !(defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)))
#   error "support only POSIX platform"
#endif

#include "predef.hpp"
extern "C" {
#   include <sys/socket.h>
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <errno.h>
#   include <microhttpd.h>
}
#include <cstdlib>
#include <cstring>
#include <new>
#include <map>
#include <boost/thread/mutex.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/unordered_map.hpp>
#include "rapidjson/document.h"
#include "stage/sys.hpp"
#include "Signer.hpp"
#include "Item.hpp"
#include "Keeper.hpp"
#include "Config.hpp"

namespace taboo
{

namespace
{

typedef std::map<std::string, std::string, std::less<std::string> > ParamMap;

class Closure
{
    static const std::string methodGet;
    static const std::string methodPost;

public:
    ParamMap params;
    std::string url, method, sign;
    MHD_PostProcessor* postProcessor;
    const Config* config;

    void destroy();

    static Closure* create(MHD_PostProcessor* postProcessor) throw();

    void recordMeta(const char* _url, const char* _method)
    {
        url = _url;
        method = _method;
    }

    void recordParam(const char* _key, const char* data, std::size_t len)
    {
        std::string key(_key);
        if (key == config->keySign) {
            sign.assign(data, len);
        } else {
            params.insert(std::make_pair(key, std::string(data, len)));
        }
    }

    bool checkSign() const
    {
        if (!config->checkSign) {
            return true;
        }
        MD5Stream stream;
        stream << url << config->signDelimiter
            << method << config->signDelimiter
            << config->manageSecret;
        for (ParamMap::const_iterator it = params.begin(); it != params.end(); ++it) {
            stream << config->signDelimiter << it->first << config->signHyphen << it->second;
        }
        return stream.hex() == sign;
    }

    bool checkParams() const
    {
        CS_DUMP(url);
        CS_DUMP(method);
        CS_DUMP(sign);
        return !url.empty() && !method.empty() && !sign.empty()
            && params.find(config->keyManageKey) != params.end()
            && params.find(config->keyPrefixes) != params.end()
            && params.find(config->keyItem) != params.end();
    }

    bool isPost() const
    {
        return method == methodPost;
    }

    ItemPtr makeItem() const
    {
        ParamMap::const_iterator it = params.find(config->keyItem);
        if (it != params.end()) {
            return taboo::makeItem(it->second.c_str());
        } else {
            return ItemPtr();
        }
    }

    KeyList getPrefixes() const
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

private:
    Closure():
        postProcessor(NULL), config(Config::instance())
    {}

    ~Closure();
};

typedef boost::singleton_pool<Closure, sizeof(Closure)> ClosurePool;

inline Closure* Closure::create(MHD_PostProcessor* postProcessor = NULL) throw()
{
    Closure* closure = new (ClosurePool::malloc()) Closure;
    closure->postProcessor = postProcessor;
    return closure;
}

inline void Closure::destroy()
{
    if (postProcessor) {
        CS_DUMP((uint64_t)postProcessor);
        MHD_destroy_post_processor(postProcessor);
    }
    ClosurePool::free(this);
}

const std::string Closure::methodGet("GET");
const std::string Closure::methodPost("POST");

}

class Manager
{
private:
    static Manager* _instance;

    typedef enum {
        ok = 0,
        error_unknown = 1,
        bad_sign = 101,
        bad_param = 102,
        no_prefixes = 10001,
        failed_on_create_item = 10002,
        failed_on_attach_prefixes = 10003,
        failed_on_attach_item = 10004,
    } ErrCode;

    typedef boost::unordered_map<ErrCode, std::string> ErrMap;
    static ErrMap errs;
    static std::string okResponse;

    static boost::mutex initMutex;

public:
    static bool initialize()
    {
        boost::mutex::scoped_lock lock(initMutex);
        if (_instance == NULL) {
            _instance = new Manager;
        }
        initializeResponse();
        return true;
    }

    static Manager* instance()
    {
        return _instance;
    }

    void start() throw()
    {
        createDaemon();
    }

protected:
    MHD_Daemon* createDaemon() throw()
    {
        const Config* config = Config::instance();
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(config->manageHost.c_str());
        addr.sin_port = htons(config->managePort);
        MHD_Daemon* daemon = MHD_start_daemon(MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY,
            config->managePort, &Manager::checkAccess, NULL,
            &Manager::handleReqpest, NULL,
            MHD_OPTION_CONNECTION_MEMORY_LIMIT, config->manageConnectionMemoryLimit,
            MHD_OPTION_CONNECTION_LIMIT, config->maxManageConnections,
            MHD_OPTION_CONNECTION_TIMEOUT, config->manageRecvTimeout,
            MHD_OPTION_SOCK_ADDR, &addr,
            MHD_OPTION_EXTERNAL_LOGGER,  &Manager::logError, NULL,
            MHD_OPTION_THREAD_POOL_SIZE, config->manageWorkers,
            MHD_OPTION_THREAD_STACK_SIZE, config->stackSize,
            MHD_OPTION_LISTENING_ADDRESS_REUSE, config->reuseAddress,
            MHD_OPTION_NOTIFY_COMPLETED, &Manager::onRequestCompleted, NULL,
            MHD_OPTION_END);
        CS_DUMP((uint64_t)daemon);
        if (daemon == NULL) {
            CS_DIE("failed on create MHD_Daemon: " << CS_LINESEP << strerror(errno));
        }
        return daemon;
    }

protected:
    static int handleReqpest(void* _closure, MHD_Connection* connection,
        const char* url,
        const char* method, const char* version,
        const char* uploadData,
        size_t* uploadDataSize, void** conClosure) throw()
    {
        if (*conClosure == NULL) {
            Closure* closure = Closure::create();
            *conClosure = closure;
            closure->recordMeta(url, method);
            MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND,
                &Manager::getParamIterator, closure);
            if (closure->isPost()) {
                closure->postProcessor = MHD_create_post_processor(connection,
                    closure->config->manageConnectionReadBuffer,
                    &Manager::postParamIterator, *conClosure);
            }
            return closure->postProcessor ? MHD_YES : MHD_NO;;

        } else if (*uploadDataSize) {
            int res = MHD_post_process(static_cast<Closure*>(*conClosure)->postProcessor, uploadData, *uploadDataSize);
            *uploadDataSize = 0;
            return res;

        } else {
            ErrCode err = handleRequest(static_cast<Closure*>(*conClosure));
            *conClosure = NULL;
            return MHD_queue_response(connection, 200, createResponse(err));
        }
    }

    static ErrCode handleRequest(Closure* closure) throw()
    {
        if (!closure->checkParams()) {
            return bad_param;
        }
        if (closure->config->checkSign && !closure->checkSign()) {
            return bad_sign;
        }
        KeyList keys = closure->getPrefixes();
        if (keys.empty()) {
            return no_prefixes;
        }
        ItemPtr item = closure->makeItem();
        if (!item) {
            return failed_on_create_item;
        }
        if (!Keeper::instance()->attach(keys, item)) {
            return failed_on_attach_item;
        }
        return ok;
    }

    static MHD_Response* createResponse(ErrCode code)
    {
        if (code == ok) {
            return MHD_create_response_from_buffer(okResponse.length(), const_cast<char*>(okResponse.data()), MHD_RESPMEM_PERSISTENT);
        } else {
            ErrMap::const_iterator it = errs.find(code);
            if (it == errs.end()) {
                it = errs.find(error_unknown);
            }
            return MHD_create_response_from_buffer(it->second.length(), const_cast<char*>(it->second.data()), MHD_RESPMEM_PERSISTENT);
        }
    }

    static int postParamIterator(void* closure, MHD_ValueKind kind, const char* key,
        const char* filename, const char* contentType, const char* transferEncoding,
        const char* data, uint64_t offset, size_t size) throw()
    {
        Closure* cls = static_cast<Closure*>(closure);
        cls->recordParam(key, data, size);
        return MHD_YES;
    }

    static int getParamIterator(void* _closure, MHD_ValueKind kind,
        const char* key, const char* value) throw()
    {
        Closure* closure = static_cast<Closure*>(_closure);
        closure->recordParam(key, value, std::strlen(value));
        return MHD_YES;
    }

    static void onRequestCompleted(void* closure, MHD_Connection* connection,
        void** conClosure, MHD_RequestTerminationCode code) throw()
    {
        if (*conClosure != NULL) {
            static_cast<Closure*>(*conClosure)->destroy();
        }
    }

    static int checkAccess(void* cls, const sockaddr* addr, socklen_t addrlen) throw()
    {
        return MHD_YES;
    }

    static void logError(void *cls, const char *fm, va_list ap) throw()
    {
        CS_DUMP("log an error");
    }

    static void initializeResponse()
    {
#define __TABOO_ADD_ERR_RESP(code, desc)  errs.insert(std::make_pair(code, std::string(             \
        "{\"result\":" + boost::lexical_cast<std::string>(code) + ",\"desc\":\"" desc "\"}")));

        __TABOO_ADD_ERR_RESP(ok, "success");
        __TABOO_ADD_ERR_RESP(error_unknown, "unknown error");
        __TABOO_ADD_ERR_RESP(bad_sign, "bad sign");
        __TABOO_ADD_ERR_RESP(bad_param, "bad param");
        __TABOO_ADD_ERR_RESP(no_prefixes, "no prefixes");
        __TABOO_ADD_ERR_RESP(failed_on_create_item, "failed on creating item");
        __TABOO_ADD_ERR_RESP(failed_on_attach_prefixes, "failed on creating prefixes");
        __TABOO_ADD_ERR_RESP(failed_on_attach_item, "failed on creating item");

#undef __TABOO_ADD_ERR_RESP

        okResponse = errs.find(ok)->second;
    }
};

}
