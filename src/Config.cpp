
#include "Config.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
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

        ("manage-workers", boost::program_options::value<std::size_t>()->default_value(1),
            "num of workers for manage, default is 1.")
        ("query-workers", boost::program_options::value<std::size_t>()->default_value(std::min<std::size_t>(1, stage::getCpuNum() - 1)),
            "num of workers for query, default is num of CPU cores minus 1.")

        ("stack-size", boost::program_options::value<std::size_t>()->default_value(stage::getRlimitCur(RLIMIT_STACK)),
            "stack size limit (KB), default not set.")
        ("memlock", boost::program_options::bool_switch()->default_value(false),
            "memlock after startup or not, default is no")
        ("max-open-files", boost::program_options::value<std::size_t>()->default_value(stage::getRlimitCur(RLIMIT_NOFILE)),
            "max open files, default not set.")
        ("reuse-address", boost::program_options::bool_switch()->default_value(true),
            "whether reuse-address on startup or not, default on.")
        ("tcp-nodelay", boost::program_options::bool_switch()->default_value(true),
            "enables tcp-nodelay feature or not, default on.")
        ("listen-backlog", boost::program_options::value<std::size_t>()->default_value(1024),
            "listen backlog, default 1024.")

        ("max-manage-connections", boost::program_options::value<std::size_t>()->default_value(10000),
            "max manage connections, default 10,000.")
        ("max-query-connections", boost::program_options::value<std::size_t>()->default_value(10000000),
            "max query connections, default 10,000,000.")

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

        ("items-allocate-step", boost::program_options::value<std::size_t>()->default_value((10 << 10)),
            "count of items to allocate for each increment, default is 10240.")
        ("max-items", boost::program_options::value<std::size_t>()->default_value((0)),
            "max count of items, 0 for unlimited, default is 0.")

        ("prefix-min-length", boost::program_options::value<std::size_t>()->default_value((2)),
            "min length for prefix of query, at least 1, default is 2.")
        ("prefix-max-length", boost::program_options::value<std::size_t>()->default_value((60)),
            "max length for prefix of query, default is 60. NOTE: it's not applied for manage requests.")

        ("max-iterations", boost::program_options::value<std::size_t>()->default_value((3000)),
            "max iterations for each matching, default is 3000.")
        ("max-matches", boost::program_options::value<std::size_t>()->default_value((100)),
            "max match count to repsonse, default is 100.")
        ("default-matches", boost::program_options::value<std::size_t>()->default_value((10)),
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
    try
    {
        boost::program_options::store(boost::program_options::parse_config_file<char>(file.c_str(), desc), options);
    }
    catch (const std::exception& e)
    {
        CS_DIE("faild on read/parse config-file: " << file << "\n" << e.what());
    }
    boost::program_options::notify(options);

    pidFile = options["pid-file"].as<boost::filesystem::path>();

    trieFile = options["trie-file"].as<boost::filesystem::path>();
    itemsFile = options["items-file"].as<boost::filesystem::path>();

    manageHost = boost::asio::ip::address_v4::from_string(options["manage-host"].as<std::string>());
    managePort = options["manage-port"].as<uint16_t>();
    queryHost = boost::asio::ip::address_v4::from_string(options["query-host"].as<std::string>());
    queryPort = options["query-port"].as<uint16_t>();

    manageWorkers = options["manage-workers"].as<std::size_t>();
    queryWorkers = options["query-workers"].as<std::size_t>();

    stackSize = options["stack-size"].as<std::size_t>() << 10;
    memlock = options["memlock"].as<bool>();
    maxOpenFiles = options["max-open-files"].as<std::size_t>();
    reuseAddress = options["reuse-address"].as<bool>();
    tcpNodelay = options["tcp-nodelay"].as<bool>();
    backlog = options["listen-backlog"].as<std::size_t>();

    maxManageConnections = options["max-manage-connections"].as<std::size_t>();
    maxQueryConnections = options["max-ws-connections"].as<std::size_t>();

    manageRecvTimeout = options["manage-receive-timeout"].as<std::time_t>();
    manageSendTimeout = options["manage-send-timeout"].as<std::time_t>();
    queryRecvTimeout = options["query-receive-timeout"].as<std::time_t>();
    querySendTimeout = options["query-send-timeout"].as<std::time_t>();

    connectionMaxIdle = options["connection-max-idle"].as<std::time_t>();
    connectionCheckInterval = options["connection-check-interval"].as<std::time_t>();

    itemsAllocStep = options["items-allocate-step"].as<std::size_t>();
    maxItems = options["max-items"].as<std::size_t>();

    prefixMinLen = options["prefix-min-len"].as<uint32_t>();
    prefixMaxLen = options["prefix-max-len"].as<uint32_t>();

    maxIterations = options["max-iterations"].as<uint32_t>();
    maxMatches = options["max-matches"].as<uint32_t>();
    defaultMatches = options["default-matches"].as<uint32_t>();

    keyId = options["id-key"].as<std::string>();
    keyPrefixes = options["id-prefixes"].as<std::string>();
    keyPrefix = options["id-prefix"].as<std::string>();
    keyFilters = options["id-filters"].as<std::string>();
    keyExcludes = options["id-excludes"].as<std::string>();
    keyFields = options["id-fields"].as<std::string>();
    keyNum = options["id-num"].as<std::string>();

    CS_SAY(
        "loaded configs in [" << file.string() << "]:" << std::endl

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

Config* Config::_instance = NULL;

}

#undef _CSOCKS_OUT_CONFIG_PROPERTY
