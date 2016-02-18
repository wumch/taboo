
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
#include "Config.hpp"
#include "Router.hpp"

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
    const Config* config;
    MHD_PostProcessor* postProcessor;
    BaseHandler* handler;

    void destroy() throw();

    static Closure* create(const std::string& method, const std::string& uri,
        MHD_PostProcessor* postProcessor = NULL) throw();

private:
    Closure():
        config(Config::instance()),
        postProcessor(NULL), handler(NULL)
    {}

    ~Closure();
};

typedef boost::singleton_pool<Closure, sizeof(Closure)> ClosurePool;

inline Closure* Closure::create(const std::string& method, const std::string& uri,
    MHD_PostProcessor* postProcessor) throw()
{
    Closure* closure = new (ClosurePool::malloc()) Closure;
    closure->handler = Router::instance()->route(method, uri).release();
    closure->postProcessor = postProcessor;
    return closure;
}

inline void Closure::destroy() throw()
{
    if (postProcessor) {
        MHD_destroy_post_processor(postProcessor);
    }
    if (handler) {
        delete handler;
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

    static boost::mutex initMutex;

public:
    static bool initialize()
    {
        boost::mutex::scoped_lock lock(initMutex);
        if (_instance == NULL) {
            _instance = new Manager;
        }
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
#if MHD_VERSION > 0x00093301
            MHD_OPTION_LISTENING_ADDRESS_REUSE, config->reuseAddress,
#endif
            MHD_OPTION_NOTIFY_COMPLETED, &Manager::onRequestCompleted, NULL,
            MHD_OPTION_END);
        if (daemon == NULL) {
            CS_DIE("failed on create MHD_Daemon: " << CS_LINESEP << strerror(errno));
        }
        return daemon;
    }

protected:
    static int handleReqpest(void* _closure, MHD_Connection* connection,
        const char* uri,
        const char* method, const char* version,
        const char* uploadData,
        size_t* uploadDataSize, void** conClosure) throw()
    {
        if (*conClosure == NULL) {
            bool ok = true;
            Closure* closure = Closure::create(method, uri);
            *conClosure = closure;
            MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND,
                &Manager::getParamIterator, closure);
            if (closure->handler->isPost()) {
                closure->postProcessor = MHD_create_post_processor(connection,
                    closure->config->manageRecvBuffer,
                    &Manager::postParamIterator, *conClosure);
                ok = closure->postProcessor != NULL;
            }
            return ok ? MHD_YES : MHD_NO;;

        } else if (*uploadDataSize) {
            int res = MHD_post_process(static_cast<Closure*>(*conClosure)->postProcessor,
                uploadData, *uploadDataSize);
            *uploadDataSize = 0;
            return res;

        } else {
            SharedReply reply = static_cast<Closure*>(*conClosure)->handler->process();
            static_cast<Closure*>(*conClosure)->destroy();
            *conClosure = NULL;
            uint32_t httpStatus;
            MHD_Response* response = createResponse(reply, httpStatus);
            return MHD_queue_response(connection, httpStatus, response);
        }
    }

    static MHD_Response* createResponse(const SharedReply& reply, uint32_t& httpStatus)
    {
        MHD_Response* response = MHD_create_response_from_buffer(reply->content.length(),
            const_cast<char*>(reply->content.data()),
            static_cast<MHD_ResponseMemoryMode>(reply->memMode));
        httpStatus = 500;
        if (CS_LIKELY(response)) {
            if (CS_BLIKELY(MHD_add_response_header(response, "Content-Type",
                "application/json") == MHD_YES)) {
                httpStatus = 200;
            }
        }
        return response;
    }

    static int postParamIterator(void* closure, MHD_ValueKind kind, const char* key,
        const char* filename, const char* contentType, const char* transferEncoding,
        const char* data, uint64_t offset, size_t size) throw()
    {
        return static_cast<Closure*>(closure)->handler->addPostParam(key, data, size) ? MHD_YES : MHD_NO;
    }

    static int getParamIterator(void* _closure, MHD_ValueKind kind,
        const char* key, const char* value) throw()
    {
        return static_cast<Closure*>(_closure)->handler->addGetParam(key, value) ? MHD_YES : MHD_NO;
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
};

}
