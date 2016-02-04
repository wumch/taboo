
#pragma once

#include "predef.hpp"
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/asio/ip/tcp.hpp>

extern int main(int, char*[]);

namespace taboo
{

class Config
{
private:
    static Config* _instance;

    Config():
        desc("allowed config options")
    {}

    friend int ::main(int, char*[]);
    static void initialize(int argc, char* argv[]);

    void init(int argc, char* argv[]);

    void initDesc();

    void load(const boost::filesystem::path& file);

    boost::program_options::variables_map options;
    boost::program_options::options_description desc;

public:
    static const Config* instance()
    {
        return _instance;
    }

    std::string programName;
    boost::filesystem::path pidFile;
    std::size_t workerCount;
    std::size_t stackSize;
    bool memlock;
    std::size_t maxOpenFiles;

    bool reuseAddress;
    std::size_t maxManagerConnections;
    std::size_t maxWsConnections;
    std::size_t backlog;
    bool tcpNodelay;

    boost::asio::ip::address host;
    uint16_t port;

    std::time_t managerRecvTimeout, managerSendTimeout,
        wsRecvTimeout, wsSendTimeout;
    std::time_t connectionMaxIdle, connectionCheckInterval;

    std::size_t maxIterations;
    std::size_t maxMatches, defaultMatches;

    std::string keyId, keyPrefix, KeyFilters, keyExcludes, keyFields;
    std::size_t itemsAllocStep, maxItems;

    std::size_t prefixMinLen, prefixMaxLen;
};

}
