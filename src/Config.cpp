
#include "Config.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <exception>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include "stage/sys.hpp"
#include "stage/net.hpp"

#define _CSOCKS_OUT_CONFIG_PROPERTY(property)     << CS_OC_GREEN(#property) << ":\t\t" << CS_OC_RED(property) << std::endl

namespace taboo
{

void Config::initialize(int argc, char* argv[])
{
    _instance = new Config;
    _instance->init(argc, argv);
}

void Config::init(int argc, char* argv[])
{
    boost::filesystem::path programPath(argv[0]);
#if BOOST_VERSION > 104200
    programName = programPath.filename().string();
#else
    programName = programPath.filename();
#endif

    boost::filesystem::path defaultConfig("etc/" + programName + ".conf");
    desc.add_options()
        ("help,h", "show this help and exit.")
        ("config,c", boost::program_options::value<boost::filesystem::path>()->default_value(defaultConfig),
            ("config file, default " + defaultConfig.string() + ".").c_str())
        ("no-config-file", boost::program_options::bool_switch()->default_value(false),
            "force do not load options from config file, default false.");
    initDesc();

    try
    {
        boost::program_options::command_line_parser parser(argc, argv);
        parser.options(desc).allow_unregistered().style(boost::program_options::command_line_style::unix_style);
        boost::program_options::store(parser.run(), options);
    }
    catch (const std::exception& e)
    {
        CS_DIE(e.what() << CS_LINESEP << desc);
    }
    boost::program_options::notify(options);

    if (options.count("help"))
    {
        std::cout << desc << std::endl;
        std::exit(EXIT_SUCCESS);
    }
    else
    {
        bool noConfigFile = options["no-config-file"].as<bool>();
        if (!noConfigFile)
        {
            load(options["config"].as<boost::filesystem::path>());
        }
    }
}

void Config::initDesc()
{
    boost::filesystem::path defaultPidFile("/var/run/" + programName + ".pid");
    boost::filesystem::path defaultStorePath("/var/lib/" + programName + "/");
    std::string lanIp = stage::getLanIP();
    desc.add_options()
        ("pid-file", boost::program_options::value<boost::filesystem::path>()->default_value(defaultPidFile),
            ("pid file, default" + defaultPidFile.string() + ".").c_str())

        ("trie-file", boost::program_options::value<boost::filesystem::path>()->default_value(defaultPidFile),
            ("file to store trie, default" + defaultStorePath.string() + "trie.dat").c_str())
        ("items-file", boost::program_options::value<boost::filesystem::path>()->default_value(defaultPidFile),
            ("file to store items, default" + defaultStorePath.string() + "items.dat").c_str())

        ("manage-host", boost::program_options::value<std::string>()->default_value(lanIp),
            ("host to bind for manage, default is " + lanIp).c_str())
        ("manage-port", boost::program_options::value<uint16_t>()->default_value(1079),
            "port to bind for manage, default is 1079")
        ("query-host", boost::program_options::value<std::string>()->default_value(lanIp),
            ("host to bind for query, default is " + lanIp).c_str())
        ("query-port", boost::program_options::value<uint16_t>()->default_value(1080),
            "port to bind for query, default is 1080")

        ("manage-workers", boost::program_options::value<std::string>()->default_value("1"),
            "num of workers for manage, default is 1.")
        ("query-workers", boost::program_options::value<std::string>()->default_value(
            boost::lexical_cast<std::string>(std::min<std::size_t>(1, stage::getCpuNum() - 1))),
            "num of workers for query, default is num of CPU cores minus 1.")

        ("stack-size", boost::program_options::value<std::string>()->default_value(
            boost::lexical_cast<std::string>(stage::getRlimitCur(RLIMIT_STACK))),
            "stack size limit, default not set.")
        ("memlock", boost::program_options::bool_switch()->default_value(false),
            "memlock after startup or not, default is no")
        ("max-open-files", boost::program_options::value<std::string>()->default_value(
            boost::lexical_cast<std::string>(stage::getRlimitCur(RLIMIT_NOFILE))),
            "max open files, default not set.")
        ("reuse-address", boost::program_options::bool_switch()->default_value(true),
            "whether reuse-address on startup or not, default on.")
        ("tcp-nodelay", boost::program_options::bool_switch()->default_value(true),
            "enables tcp-nodelay feature or not, default on.")
        ("listen-backlog", boost::program_options::value<std::string>()->default_value("1k"),
            "listen backlog, default is 1k.")

        ("max-manage-connections", boost::program_options::value<std::string>()->default_value("10K"),
            "max manage connections, default 10K.")
        ("max-query-connections", boost::program_options::value<std::string>()->default_value("1M"),
            "max query connections, default 1M.")

        ("manage-receive-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "receive-timeout for manage requests (second), 0 is unlimited, default is 30s.")
        ("manage-send-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "send-timeout manage requests (second), 0 is unlimited, default is 30s.")
        ("query-receive-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "receive-timeout for query requests (second), 0 for unlimited, default is 30s.")
        ("query-send-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "send-timeout for query requests (second), 0 is unlimited, default is 30s.")

        ("connection-max-idle", boost::program_options::value<std::time_t>()->default_value((7200)),
            "max idle time for each connection (second), default is 7200.")
        ("connection-check-interval", boost::program_options::value<std::time_t>()->default_value((600)),
            "connection idle time check interval (second), default is 600.")

        ("items-allocate-step", boost::program_options::value<std::string>()->default_value(("10K")),
            "count of items to allocate for each increment, default is 10K.")
        ("max-items", boost::program_options::value<std::string>()->default_value(("0")),
            "max count of items, 0 for unlimited, default is 0.")

        ("prefix-min-length", boost::program_options::value<uint32_t>()->default_value((2)),
            "min length for prefix of query, at least 1, default is 2.")
        ("prefix-max-length", boost::program_options::value<uint32_t>()->default_value((60)),
            "max length for prefix of query, default is 60. NOTE: it's not applied for manage requests.")

        ("max-iterations", boost::program_options::value<std::string>()->default_value(("3000")),
            "max iterations for each matching, default is 3000.")
        ("max-matches", boost::program_options::value<std::string>()->default_value(("100")),
            "max match count to repsonse, default is 100.")
        ("default-matches", boost::program_options::value<std::string>()->default_value(("10")),
            "default match count to repsonse, default is 10.")

        ("key-id", boost::program_options::value<std::string>()->default_value(("id")),
            "key name for 'id' of items, default is 'id'.")
        ("key-prefixes", boost::program_options::value<std::string>()->default_value(("prefixes")),
            "key name for 'prefixes' of items in manage requests, default is 'prefixes'.")

        ("key-prefix", boost::program_options::value<std::string>()->default_value(("prefix")),
            "key name for 'prefix' of query, default is 'prefix'.")
        ("key-filters", boost::program_options::value<std::string>()->default_value(("filters")),
            "key name for 'filters'' of query, default is 'filters'.")
        ("key-excludes", boost::program_options::value<std::string>()->default_value(("excludes")),
            "key name for 'excludes'' of query, default is 'excludes'.")
        ("key-fields", boost::program_options::value<std::string>()->default_value(("fields")),
            "key name for 'fields'' of query, default is 'fields'.")
        ("key-num", boost::program_options::value<std::string>()->default_value(("num")),
            "key name for 'num'' of query, default is 'num'.")
    ;
}

void Config::load(const boost::filesystem::path& file)
{
    try {
        boost::program_options::store(boost::program_options::parse_config_file<char>(file.c_str(), desc), options);
    } catch (const std::exception& e) {
        boost::system::error_code err;
        boost::filesystem::path path = boost::filesystem::canonical(file, err);
        CS_DIE("faild on load config-file: " << (err ? file : path) << "\n" << CS_OC_RED(e.what()));
    }
    boost::program_options::notify(options);

    try {
        loadValues(file);
    } catch (const std::exception& e) {
        CS_DIE("failed on load config options: " << CS_OC_RED(e.what()));
    }
}

void Config::loadValues(const boost::filesystem::path& file)
{
    pidFile = options["pid-file"].as<boost::filesystem::path>();

    trieFile = options["trie-file"].as<boost::filesystem::path>();
    itemsFile = options["items-file"].as<boost::filesystem::path>();

    manageHost = boost::asio::ip::address_v4::from_string(options["manage-host"].as<std::string>());
    managePort = options["manage-port"].as<uint16_t>();
    queryHost = boost::asio::ip::address_v4::from_string(options["query-host"].as<std::string>());
    queryPort = options["query-port"].as<uint16_t>();

    manageWorkers = toInteger<uint32_t>(options["manage-workers"].as<std::string>());
    queryWorkers = toInteger<uint32_t>(options["query-workers"].as<std::string>());

    stackSize = toInteger<std::size_t>(options["stack-size"].as<std::string>());
    memlock = options["memlock"].as<bool>();
    maxOpenFiles = toInteger<std::size_t>(options["max-open-files"].as<std::string>());
    reuseAddress = options["reuse-address"].as<bool>();
    tcpNodelay = options["tcp-nodelay"].as<bool>();
    backlog = toInteger<uint32_t>(options["listen-backlog"].as<std::string>());

    maxManageConnections = toInteger<std::size_t>(options["max-manage-connections"].as<std::string>());
    maxQueryConnections = toInteger<std::size_t>(options["max-query-connections"].as<std::string>());

    manageRecvTimeout = options["manage-receive-timeout"].as<std::time_t>();
    manageSendTimeout = options["manage-send-timeout"].as<std::time_t>();
    queryRecvTimeout = options["query-receive-timeout"].as<std::time_t>();
    querySendTimeout = options["query-send-timeout"].as<std::time_t>();

    connectionMaxIdle = options["connection-max-idle"].as<std::time_t>();
    connectionCheckInterval = options["connection-check-interval"].as<std::time_t>();

    itemsAllocStep = toInteger<std::size_t>(options["items-allocate-step"].as<std::string>());
    maxItems = toInteger<std::size_t>(options["max-items"].as<std::string>());

    prefixMinLen = options["prefix-min-length"].as<uint32_t>();
    prefixMaxLen = options["prefix-max-length"].as<uint32_t>();

    maxIterations = toInteger<uint32_t>(options["max-iterations"].as<std::string>());
    maxMatches = toInteger<uint32_t>(options["max-matches"].as<std::string>());
    defaultMatches = toInteger<uint32_t>(options["default-matches"].as<std::string>());

    keyId = options["key-id"].as<std::string>();
    keyPrefixes = options["key-prefixes"].as<std::string>();
    keyPrefix = options["key-prefix"].as<std::string>();
    keyFilters = options["key-filters"].as<std::string>();
    keyExcludes = options["key-excludes"].as<std::string>();
    keyFields = options["key-fields"].as<std::string>();
    keyNum = options["key-num"].as<std::string>();

#if CS_DEBUG
    boost::system::error_code err;
    boost::filesystem::path path = boost::filesystem::canonical(file, err);
#endif
    CS_SAY(
        "loaded configs in [" << (err ? file : path) << "]:" << std::endl

        _CSOCKS_OUT_CONFIG_PROPERTY(programName)
        _CSOCKS_OUT_CONFIG_PROPERTY(manageHost)
        _CSOCKS_OUT_CONFIG_PROPERTY(managePort)
        _CSOCKS_OUT_CONFIG_PROPERTY(queryHost)
        _CSOCKS_OUT_CONFIG_PROPERTY(queryPort)
        _CSOCKS_OUT_CONFIG_PROPERTY(pidFile)
        _CSOCKS_OUT_CONFIG_PROPERTY(queryWorkers)
        _CSOCKS_OUT_CONFIG_PROPERTY(stackSize)
        _CSOCKS_OUT_CONFIG_PROPERTY(memlock)
        _CSOCKS_OUT_CONFIG_PROPERTY(maxOpenFiles)
        _CSOCKS_OUT_CONFIG_PROPERTY(reuseAddress)
        _CSOCKS_OUT_CONFIG_PROPERTY(tcpNodelay)
        _CSOCKS_OUT_CONFIG_PROPERTY(backlog)

        _CSOCKS_OUT_CONFIG_PROPERTY(maxManageConnections)
        _CSOCKS_OUT_CONFIG_PROPERTY(maxQueryConnections)

        _CSOCKS_OUT_CONFIG_PROPERTY(queryRecvTimeout)
        _CSOCKS_OUT_CONFIG_PROPERTY(querySendTimeout)
        _CSOCKS_OUT_CONFIG_PROPERTY(manageRecvTimeout)
        _CSOCKS_OUT_CONFIG_PROPERTY(manageSendTimeout)

        _CSOCKS_OUT_CONFIG_PROPERTY(connectionMaxIdle)
        _CSOCKS_OUT_CONFIG_PROPERTY(connectionCheckInterval)

        _CSOCKS_OUT_CONFIG_PROPERTY(itemsAllocStep)
        _CSOCKS_OUT_CONFIG_PROPERTY(maxItems)

        _CSOCKS_OUT_CONFIG_PROPERTY(maxIterations)
        _CSOCKS_OUT_CONFIG_PROPERTY(maxMatches)
        _CSOCKS_OUT_CONFIG_PROPERTY(defaultMatches)

        _CSOCKS_OUT_CONFIG_PROPERTY(keyId)
        _CSOCKS_OUT_CONFIG_PROPERTY(keyPrefixes)
        _CSOCKS_OUT_CONFIG_PROPERTY(keyPrefix)
        _CSOCKS_OUT_CONFIG_PROPERTY(keyFilters)
        _CSOCKS_OUT_CONFIG_PROPERTY(keyExcludes)
        _CSOCKS_OUT_CONFIG_PROPERTY(keyFields)
        _CSOCKS_OUT_CONFIG_PROPERTY(keyNum)

        _CSOCKS_OUT_CONFIG_PROPERTY(prefixMinLen)
        _CSOCKS_OUT_CONFIG_PROPERTY(prefixMaxLen)
    );
}

template<typename IntType> IntType Config::toInteger(const std::string& str) const
{
    int unit = *str.rbegin();
    int bits = 0;
    if (std::isalpha(unit)) {
        char u = std::tolower(unit);
        switch (u) {
        case 'k': bits = 10; break;
        case 'm': bits = 20; break;
        case 'g': bits = 30; break;
        case 't': bits = 40; break;
        default: bits = -1; break;
        }
    } else {
        return boost::lexical_cast<IntType>(str);
    }
    if (bits == -1) {
        throw std::logic_error("bad value: " + str);
    }
    IntType num = boost::lexical_cast<IntType>(str.substr(0, str.length() - 1));
    int bitsRemain = sizeof(IntType) - bits;
    IntType max = 1 << bitsRemain;
    if (!(num < max)) {
        throw std::logic_error("value out of bound: " + str);
    }
    if (num < 0 && 0 < static_cast<IntType>(-1)) {
        throw std::logic_error("positive value required, but got " + str);
    }
    return num << bits;
}

Config* Config::_instance = NULL;

}

#undef _CSOCKS_OUT_CONFIG_PROPERTY
