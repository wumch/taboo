
#pragma once

#include "predef.hpp"
#include <string>
#include <boost/thread/mutex.hpp>
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
        desc("allowed options")
    {}

    static boost::mutex configLoadMutex;
    friend int ::main(int, char*[]);
    static bool initialize(int argc, char* argv[]);

    void init(int argc, char* argv[]);

    void initDesc();

    void loadFile();

    void loadOptions();

    template<typename IntType> IntType toInteger(const std::string& name) const;
    template<typename T> T to(const std::string& name) const;

    bool purposeShowHelp, purposeTestConfig, purposeReloadConfig;

    bool noFile;
    boost::filesystem::path file;
    boost::program_options::variables_map options;
    boost::program_options::options_description desc;

public:
    static const Config* instance()
    {
        return _instance;
    }

    std::string programName;

    boost::filesystem::path pidFile;

    bool storeOnExit, restoreOnStart;
    boost::filesystem::path trieFile, itemsFile;

    std::string manageHost, queryHost;
    uint16_t managePort, queryPort;

    uint32_t manageWorkers, queryWorkers;

    std::size_t stackSize;
    std::size_t maxOpenFiles;
    bool memlock;
    bool reuseAddress;
    bool tcpNodelay;
    uint32_t backlog;

    std::size_t maxManageConnections;
    std::size_t maxQueryConnections;

    std::size_t manageConnectionMemoryLimit, manageConnectionReadBuffer;
    std::time_t manageRecvTimeout, manageSendTimeout,
        queryRecvTimeout, querySendTimeout;
    std::time_t connectionMaxIdle, connectionCheckInterval;

    std::size_t itemsAllocStep, maxItems;

    uint32_t prefixMinLen, prefixMaxLen;

    uint32_t maxIterations, maxMatches, defaultMatches;

    bool checkSign;
    std::string manageKey, manageSecret, signHyphen, signDelimiter;

    std::string keySign, keyManageKey,
        keyId, keyPrefixes, keyPrefix, keyFilters, keyExcludes, keyFields, keyNum;
};

}
