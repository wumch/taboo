
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
    friend int ::main(int, char*[]);
private:
    static Config _instance;

    Config():
        desc("allowed config options")
    {}

    void load(boost::filesystem::path file);

    static Config* mutableInstance()
    {
        return &_instance;
    }

    void initDesc();

public:
    static const Config* instance()
    {
        return &_instance;
    }

    void init(int argc, char* argv[]);

public:
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

    std::string idKey;
    std::size_t itemsAllocStep, maxItems;

    boost::program_options::variables_map options;
    boost::program_options::options_description desc;
};

}
