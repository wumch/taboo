
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
#include <boost/pool/singleton_pool.hpp>
#include "stage/sys.hpp"
#include "Config.hpp"
#include "Signer.hpp"

namespace taboo
{

namespace
{

class Closure
{
public:
    MD5Stream md5stream;
    MHD_PostProcessor* postProcessor;
    std::string sign;

    void destroy();

    static Closure* create(MHD_PostProcessor* postProcessor) throw();

private:
    Closure():
        postProcessor(NULL)
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

}

class Manager
{
private:
    static Manager* _instance;

    MD5Stream md5Stream;

public:
    static bool initialize()
    {
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
            config->managePort, &Manager::check, NULL,
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
    static int handleReqpest(void *closure, MHD_Connection *connection,
        const char *url,
        const char *method, const char *version,
        const char *uploadData,
        size_t *uploadDataSize, void **conClosure) throw()
    {
        if (*conClosure == NULL) {
            CS_SAY("first");
            Closure* cls = Closure::create();
            *conClosure = cls;
            const Config* config = Config::instance();
            cls->md5stream << config->manageSecret;
            return (cls->postProcessor = MHD_create_post_processor(connection,
                Config::instance()->manageConnectionReadBuffer,
                &Manager::postDataIterator, *conClosure)) ? MHD_YES : MHD_NO;;
        } else if (*uploadDataSize) {
            int res = MHD_post_process(static_cast<Closure*>(*conClosure)->postProcessor, uploadData, *uploadDataSize);
            *uploadDataSize = 0;
            return res;
        } else {
            char* content;
            Closure* cls = static_cast<Closure*>(*conClosure);
            std::string sign = cls->md5stream.hex();
            if (sign != cls->sign) {
                CS_DUMP(sign);
                CS_DUMP(cls->sign);
                content = "{\"result\":\"10001\",\"desc\":\"bad sign\"}";
            } else {
                CS_SAY("done");
    //            MHD_destroy_post_processor(static_cast<Closure*>(*conClosure)->postProcessor);
                content = "haha";
            }
            MHD_Response* response = MHD_create_response_from_buffer(
                strlen(content), content, MHD_RESPMEM_MUST_COPY);
            cls->destroy();
            *conClosure = NULL;
            return MHD_queue_response(connection, 200, response);
        }
        MHD_get_connection_values(connection,
            static_cast<MHD_ValueKind>(MHD_GET_ARGUMENT_KIND | MHD_POSTDATA_KIND),
            NULL, NULL);
        return MHD_YES;
    }

    static int postDataIterator(void* closure, MHD_ValueKind kind, const char* key,
        const char* filename, const char* contentType, const char* transferEncoding,
        const char* data, uint64_t offset, size_t size) throw()
    {
        CS_DUMP(key);
        CS_DUMP(offset);
        CS_DUMP(size);
        CS_DUMP(std::string(data, size));

        Closure* cls = static_cast<Closure*>(closure);
        if (key == Config::instance()->keySign) {
            cls->sign.assign(data, size);
        } else {
            cls->md5stream << Config::instance()->signDelimiter << key << Config::instance()->signHyphen << StrRef(data, size);
        }
        return MHD_YES;
    }

    static void onRequestCompleted(void* closure, MHD_Connection* connection,
        void** conClosure, MHD_RequestTerminationCode code) throw()
    {
        if (*conClosure != NULL) {
            static_cast<Closure*>(*conClosure)->destroy();
        }
    }

    static int check(void* cls, const sockaddr* addr, socklen_t addrlen) throw()
    {
        CS_DUMP(inet_ntoa(reinterpret_cast<const sockaddr_in*>(addr)->sin_addr));
        return MHD_YES;
    }

    static void logError(void *cls, const char *fm, va_list ap) throw()
    {
        CS_DUMP("log error");
    }
};

}
