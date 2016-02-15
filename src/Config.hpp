
#pragma once

#include "predef.hpp"
#include <ctime>
#include <string>
#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

extern int main(int, char*[]);

namespace po = boost::program_options;

namespace taboo {

typedef std::vector<std::string> StringList;

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

    template<typename T> T to(const std::string& name) const;

    template<typename T> std::vector<T> series(const std::string& name) const;

    bool purposeShowHelp, purposeTestConfig, purposeReloadConfig;

    bool noFile;
    boost::filesystem::path defaultConfigDir, configFile;
    po::variables_map options;
    po::options_description desc;

public:
    static const Config* instance()
    {
        return _instance;
    }

    std::string programName;

    boost::filesystem::path pidFile;

    bool storeOnExit, restoreOnStart;
    boost::filesystem::path trieFile, itemsFile;

    bool queryEnableHttp, queryEnableHttps, queryEnableWs, queryEnableWss;
    boost::filesystem::path wssCert, httpsCert;

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

    std::size_t manageConnectionMemoryLimit,
        manageRecvBuffer, manageSendBuffer,
        queryRecvBuffer, querySendBuffer;
    std::time_t manageRecvTimeout, manageSendTimeout,
        queryRecvTimeout, querySendTimeout;
    std::time_t connectionMaxIdle, connectionCheckInterval;

    std::size_t itemsAllocStep, maxItems;

    uint32_t prefixMinLen, prefixMaxLen;
    std::size_t queryDataMaxSize;

    uint32_t maxIterations, maxMatches, defaultMatches;

    bool checkSign, manageMustPost;
    std::string manageKey, manageSecret, signHyphen, signDelimiter;

    std::time_t tokenDefaultExpire;

    std::string keyMUKey, keyMUSign, keyMUPrefixes, keyMUItem, keyMUUpsert,
        keyMUIdenty, keyMUFilters, keyMUExpire,
        keyMDErrCode, keyMDErrDesc, keyMDPayload, keyMDToken, keyMDTokenExpire,
        keyId,
        keyQEchoData,
        keyQUPayload, keyQUToken, keyQUPrefix, keyQUFilters, keyQUExcludes, keyQUFields, keyQUNum,
        keyQDErrCode, keyQDErrDesc, keyQDPayload;

    StringList queryVisibleFields, queryInvisibleFields;
    bool queryVisibleAll;
};

}
