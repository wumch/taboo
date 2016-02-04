
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

    boost::filesystem::path trieFile, itemsFile;

    boost::asio::ip::address manageHost, queryHost;
    uint16_t managePort, queryPort;

    std::size_t manageWorkers, queryWorkers;

    std::size_t stackSize;
    bool memlock;
    std::size_t maxOpenFiles;
    bool reuseAddress;
    bool tcpNodelay;
    std::size_t backlog;

    std::size_t maxManageConnections;
    std::size_t maxQueryConnections;

    std::time_t manageRecvTimeout, manageSendTimeout,
        queryRecvTimeout, querySendTimeout;
    std::time_t connectionMaxIdle, connectionCheckInterval;

    std::size_t itemsAllocStep, maxItems;

    uint32_t prefixMinLen, prefixMaxLen;

    uint32_t maxIterations, maxMatches, defaultMatches;

    std::string keyId, keyPrefixes, keyPrefix, keyFilters, keyExcludes, keyFields, keyNum;
};

}
