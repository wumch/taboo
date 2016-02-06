
#pragma once

#include "predef.hpp"
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

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
    static bool initialize(int argc, char* argv[]);

    void init(int argc, char* argv[]);

    void initDesc();

    void load(const boost::filesystem::path& file);

    void loadOptions(const boost::filesystem::path& file);

    template<typename IntType> IntType toInteger(const std::string& str) const;

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

    std::string manageHost, queryHost;
    uint16_t managePort, queryPort;

    uint32_t manageWorkers, queryWorkers;

    std::size_t stackSize;
    std::size_t maxOpenFiles;
    bool memlock;
    bool reuseAddress;
    bool tcpNodelay;
    std::size_t backlog;

    std::size_t maxManageConnections;
    std::size_t maxQueryConnections;

    std::size_t manageConnectionMemoryLimit, manageConnectionReadBuffer;
    std::time_t manageRecvTimeout, manageSendTimeout,
        queryRecvTimeout, querySendTimeout;
    std::time_t connectionMaxIdle, connectionCheckInterval;

    std::size_t itemsAllocStep, maxItems;

    uint32_t prefixMinLen, prefixMaxLen;

    uint32_t maxIterations, maxMatches, defaultMatches;

    std::string manageKey, manageSecret, signHyphen, signDelimiter;

    std::string keySign, keyId, keyPrefixes, keyPrefix, keyFilters, keyExcludes, keyFields, keyNum;
};

}
