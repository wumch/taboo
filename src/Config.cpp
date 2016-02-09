
#include "Config.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <exception>
#include <memory>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include "stage/sys.hpp"
#include "stage/net.hpp"

#define _TABOO_OUT_CONFIG_OPTION(property)     << CS_OC_GREEN(#property)         \
    << ":\t\t" << CS_OC_RED(property) << std::endl

namespace taboo
{

bool Config::initialize(int argc, char* argv[])
{
    typedef std::auto_ptr<Config> ConfigPtr;
    {
        try {
            boost::mutex::scoped_lock lock(configLoadMutex);
            ConfigPtr ptr(new Config);
            ptr->init(argc, argv);
            _instance = ptr.release();
        } catch (const std::logic_error& e) {
            CS_DIE("error: " << e.what());
        } catch (const std::runtime_error& e) {
            CS_DIE("error: " << e.what());
        } catch (const std::exception& e) {
            CS_DIE("error: " << e.what());
        }
    }

    if (_instance->purposeShowHelp || _instance->purposeTestConfig) {
        std::exit(EXIT_SUCCESS);
    }

    return true;
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
        ("test-config,t", "test config options and exit.")
        ("reload-config,r", "reload config options from previous loaded config file. "
            "(NOTE: new specified command line options are omitted, and "
            "old command line options will be overridden by ones from config file)")
        ("config,c", boost::program_options::value<boost::filesystem::path>()->default_value(defaultConfig),
            ("config file, default " + defaultConfig.string() + ". "
                "options from config file will be overridden by ones from command line.").c_str())
        ("no-config-file", boost::program_options::bool_switch()->default_value(false),
            "force do not load options from config file, default false.");
    initDesc();

    try {
        boost::program_options::command_line_parser parser(argc, argv);
        parser.options(desc).style(boost::program_options::command_line_style::unix_style);
        boost::program_options::store(parser.run(), options);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string(e.what()) + CS_LINESEP
            + CS_OC_BLACK_BEGIN + boost::lexical_cast<std::string>(desc) + CS_OC_END);
    }
    boost::program_options::notify(options);

    if (options.count("manage-key") || options.count("manage-secret")) {
        CS_ERR("warning: specify 'manage-key' or 'manage-secret' in command line is dangerous.");
    }

    purposeShowHelp = options.count("help");
    purposeTestConfig = options.count("test-config");
    purposeReloadConfig = options.count("reload-config");

    if (purposeShowHelp) {
        std::cout
            << "usage: " << argv[0] << " [options]" << CS_LINESEP
            << programName << " is an HTTP/WebSocket based prefix predict server with "
                "user-customizeable property " << CS_LINESEP
                << "matching feature. See " << TABOO_HOME_PAGE << " for more information."
            << CS_LINESEP << CS_LINESEP
            << desc << CS_LINESEP;
    } else {
        noFile = to<bool>("no-config-file");
        if (noFile) {
            loadOptions();
        } else {
            file = to<boost::filesystem::path>("config");
            loadFile();
        }
    }

    if (purposeTestConfig) {
        CS_ECHO(CS_OC_GREEN("config " << (file.empty() ? "" :
            (std::string("file '") + file.string() + "'")) << " is ok."));
    }
}

void Config::initDesc()
{
    boost::filesystem::path defaultPidFile("/var/run/" + programName + ".pid");
    boost::filesystem::path defaultStorePath("/var/lib/" + programName + "/");
    boost::filesystem::path defaultTrieFile = defaultStorePath.string() + "trie.dat";
    boost::filesystem::path defaultItemsFile = defaultStorePath.string() + "items.dat";
    std::string lanIp = stage::getLanIP();
    desc.add_options()
        ("pid-file", boost::program_options::value<boost::filesystem::path>()->default_value(defaultPidFile),
            ("pid file, default" + defaultPidFile.string() + ".").c_str())

        ("store-on-exit", boost::program_options::bool_switch()->default_value(true),
            "store trie/items into @trie-file/@items-file before exit or not, default is yes.")
        ("restore-on-start", boost::program_options::bool_switch()->default_value(true),
            "restore trie/items from @trie-file/@items-file when startup or not, default is yes.")

        ("trie-file", boost::program_options::value<boost::filesystem::path>()->default_value(defaultTrieFile),
            ("file to store trie, default is '" + defaultStorePath.string() + "trie.dat'.").c_str())
        ("items-file", boost::program_options::value<boost::filesystem::path>()->default_value(defaultItemsFile),
            ("file to store items, default is '" + defaultStorePath.string() + "items.dat'.").c_str())

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

        ("stack-size", boost::program_options::value<std::string>()->default_value("0"),
            "stack size limit, 0 is not set, default is 0.")
        ("max-open-files", boost::program_options::value<std::string>()->default_value("0"),
            "max open files, 0 is not set, default is 0.")
        ("memlock", boost::program_options::bool_switch()->default_value(false),
            "memlock after startup or not, default is no")
        ("reuse-address", boost::program_options::bool_switch()->default_value(true),
            "whether reuse-address on startup or not, default on.")
        ("tcp-nodelay", boost::program_options::bool_switch()->default_value(true),
            "enables tcp-nodelay feature or not, default on.")
        ("listen-backlog", boost::program_options::value<std::string>()->default_value("1K"),
            "listen backlog, default is 1K.")

        ("manage-connection-memory-limit", boost::program_options::value<std::string>()->default_value("256K"),
            "memory size limit per manage connection, 0 is unlimited, default is 256K.")
        ("manage-connection-read-buffer", boost::program_options::value<std::string>()->default_value("128K"),
            "size of read buffer for manage connections, default is 128K.")
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
            "max length for prefix of query, default is 60. NOTE: this is not applied for manage requests.")

        ("max-iterations", boost::program_options::value<std::string>()->default_value(("3000")),
            "max iterations for each matching, default is 3000.")
        ("max-matches", boost::program_options::value<std::string>()->default_value(("100")),
            "max count of items to match for query, default is 100.")
        ("default-matches", boost::program_options::value<std::string>()->default_value(("10")),
            "default count of items to match for query, default is 10.")

        ("check-signature", boost::program_options::bool_switch()->default_value(true),
            "check signature or not for manage requests, default is yes.")
        ("manage-key", boost::program_options::value<std::string>(),
            "access key for manage, REQUIRED when @check-signature='yes'.")
        ("manage-secret", boost::program_options::value<std::string>(),
            "access secret for manage, REQUIRED when @check-signature='yes'.")
        ("sign-hyphen", boost::program_options::value<std::string>()->default_value("="),
            "glue for join key and value when generating signature, default is '='.")
        ("sign-delimiter", boost::program_options::value<std::string>()->default_value("&"),
            "glue for join key-value pairs when generate signature, default is '&'.")

        ("key-manage-key", boost::program_options::value<std::string>()->default_value(("key")),
            "key name for 'manage-key' of manage requests, default is 'key'.")
        ("key-sign", boost::program_options::value<std::string>()->default_value(("sign")),
            "key name for 'sign' of manage requests, default is 'sign'.")
        ("key-item", boost::program_options::value<std::string>()->default_value(("item")),
            "key name for 'item' of manage requests, default is 'item'.")
        ("key-prefixes", boost::program_options::value<std::string>()->default_value(("prefixes")),
            "key name for 'prefixes' of items in manage requests, default is 'prefixes'.")
        ("key-id", boost::program_options::value<std::string>()->default_value(("id")),
            "key name for 'id' of items, default is 'id'.")

        ("key-prefix", boost::program_options::value<std::string>()->default_value(("prefix")),
            "key name for 'prefix' of query requests, default is 'prefix'.")
        ("key-filters", boost::program_options::value<std::string>()->default_value(("filters")),
            "key name for 'filters' of query requests, default is 'filters'.")
        ("key-excludes", boost::program_options::value<std::string>()->default_value(("excludes")),
            "key name for 'excludes' of query requests, default is 'excludes'.")
        ("key-fields", boost::program_options::value<std::string>()->default_value(("fields")),
            "key name for 'fields' of query requests, default is 'fields'.")
        ("key-num", boost::program_options::value<std::string>()->default_value(("num")),
            "key name for 'num' of query requests, default is 'num'.")
    ;
}

void Config::loadFile()
{
    try {
        boost::program_options::store(boost::program_options::parse_config_file<char>(file.c_str(), desc), options);
    } catch (const std::exception& e) {
        boost::system::error_code err;
        boost::filesystem::path path = boost::filesystem::canonical(file, err);
        throw std::runtime_error("faild on load config-file: "
            + (err ? file : path).string() + CS_LINESEP_STR + e.what());
    }
    boost::program_options::notify(options);

    loadOptions();
}

void Config::loadOptions()
{
    pidFile = to<boost::filesystem::path>("pid-file");

    storeOnExit = to<bool>("store-on-exit");
    restoreOnStart = to<bool>("restore-on-start");

    trieFile = to<boost::filesystem::path>("trie-file");
    itemsFile = to<boost::filesystem::path>("items-file");

    manageHost = to<std::string>("manage-host");
    managePort = to<uint16_t>("manage-port");
    queryHost = to<std::string>("query-host");
    queryPort = to<uint16_t>("query-port");

    manageWorkers = toInteger<uint32_t>("manage-workers");
    queryWorkers = toInteger<uint32_t>("query-workers");

    stackSize = toInteger<std::size_t>("stack-size");
    maxOpenFiles = toInteger<std::size_t>("max-open-files");
    memlock = to<bool>("memlock");
    reuseAddress = to<bool>("reuse-address");
    tcpNodelay = to<bool>("tcp-nodelay");
    backlog = toInteger<uint32_t>("listen-backlog");

    manageConnectionMemoryLimit = toInteger<std::size_t>("manage-connection-memory-limit");
    manageConnectionReadBuffer = toInteger<std::size_t>("manage-connection-read-buffer");
    maxManageConnections = toInteger<std::size_t>("max-manage-connections");
    maxQueryConnections = toInteger<std::size_t>("max-query-connections");

    manageRecvTimeout = to<std::time_t>("manage-receive-timeout");
    manageSendTimeout = to<std::time_t>("manage-send-timeout");
    queryRecvTimeout = to<std::time_t>("query-receive-timeout");
    querySendTimeout = to<std::time_t>("query-send-timeout");

    connectionMaxIdle = to<std::time_t>("connection-max-idle");
    connectionCheckInterval = to<std::time_t>("connection-check-interval");

    itemsAllocStep = toInteger<std::size_t>("items-allocate-step");
    maxItems = toInteger<std::size_t>("max-items");

    prefixMinLen = to<uint32_t>("prefix-min-length");
    prefixMaxLen = to<uint32_t>("prefix-max-length");

    maxIterations = toInteger<uint32_t>("max-iterations");
    maxMatches = toInteger<uint32_t>("max-matches");
    defaultMatches = toInteger<uint32_t>("default-matches");

    if (options.find("manage-key") == options.end()) {
        throw std::logic_error("config option 'manage-key' is required");
    }
    if (options.find("manage-secret") == options.end()) {
        throw std::logic_error("config option 'manage-secret' is required");
    }
    checkSign = to<bool>("check-signature");
    if (checkSign) {
        manageKey = to<std::string>("manage-key");
        manageSecret = to<std::string>("manage-secret");
        if (manageKey.empty() || manageSecret.empty()) {
            throw std::logic_error("config options 'manage-key' and "
                "'manage-secret' must not be empty");
        }
    }
    signHyphen = to<std::string>("sign-hyphen");
    signDelimiter = to<std::string>("sign-delimiter");

    keyManageKey = to<std::string>("key-manage-key");
    keySign = to<std::string>("key-sign");
    keyItem = to<std::string>("key-item");
    keyPrefixes = to<std::string>("key-prefixes");
    keyId = to<std::string>("key-id");

    keyPrefix = to<std::string>("key-prefix");
    keyFilters = to<std::string>("key-filters");
    keyExcludes = to<std::string>("key-excludes");
    keyFields = to<std::string>("key-fields");
    keyNum = to<std::string>("key-num");

#if CS_DEBUG
    boost::system::error_code err;
    boost::filesystem::path path = boost::filesystem::canonical(file, err);
#endif
    CS_SAY(
        "loaded configs in [" << (err ? file : path) << "]:" << CS_LINESEP

        _TABOO_OUT_CONFIG_OPTION(programName)
        _TABOO_OUT_CONFIG_OPTION(file)
        _TABOO_OUT_CONFIG_OPTION(pidFile)

        _TABOO_OUT_CONFIG_OPTION(storeOnExit)
        _TABOO_OUT_CONFIG_OPTION(restoreOnStart)
        _TABOO_OUT_CONFIG_OPTION(trieFile)
        _TABOO_OUT_CONFIG_OPTION(itemsFile)

        _TABOO_OUT_CONFIG_OPTION(manageHost)
        _TABOO_OUT_CONFIG_OPTION(managePort)
        _TABOO_OUT_CONFIG_OPTION(queryHost)
        _TABOO_OUT_CONFIG_OPTION(queryPort)
        _TABOO_OUT_CONFIG_OPTION(pidFile)
        _TABOO_OUT_CONFIG_OPTION(queryWorkers)
        _TABOO_OUT_CONFIG_OPTION(stackSize)
        _TABOO_OUT_CONFIG_OPTION(memlock)
        _TABOO_OUT_CONFIG_OPTION(maxOpenFiles)
        _TABOO_OUT_CONFIG_OPTION(reuseAddress)
        _TABOO_OUT_CONFIG_OPTION(tcpNodelay)
        _TABOO_OUT_CONFIG_OPTION(backlog)

        _TABOO_OUT_CONFIG_OPTION(manageConnectionMemoryLimit)
        _TABOO_OUT_CONFIG_OPTION(manageConnectionReadBuffer)
        _TABOO_OUT_CONFIG_OPTION(maxManageConnections)
        _TABOO_OUT_CONFIG_OPTION(maxQueryConnections)

        _TABOO_OUT_CONFIG_OPTION(queryRecvTimeout)
        _TABOO_OUT_CONFIG_OPTION(querySendTimeout)
        _TABOO_OUT_CONFIG_OPTION(manageRecvTimeout)
        _TABOO_OUT_CONFIG_OPTION(manageSendTimeout)

        _TABOO_OUT_CONFIG_OPTION(connectionMaxIdle)
        _TABOO_OUT_CONFIG_OPTION(connectionCheckInterval)

        _TABOO_OUT_CONFIG_OPTION(itemsAllocStep)
        _TABOO_OUT_CONFIG_OPTION(maxItems)

        _TABOO_OUT_CONFIG_OPTION(maxIterations)
        _TABOO_OUT_CONFIG_OPTION(maxMatches)
        _TABOO_OUT_CONFIG_OPTION(defaultMatches)

        _TABOO_OUT_CONFIG_OPTION(checkSign)
        _TABOO_OUT_CONFIG_OPTION(manageKey)
        _TABOO_OUT_CONFIG_OPTION(manageSecret)
        _TABOO_OUT_CONFIG_OPTION(signHyphen)
        _TABOO_OUT_CONFIG_OPTION(signDelimiter)
        _TABOO_OUT_CONFIG_OPTION(keyManageKey)
        _TABOO_OUT_CONFIG_OPTION(keySign)
        _TABOO_OUT_CONFIG_OPTION(keyItem)
        _TABOO_OUT_CONFIG_OPTION(keyId)

        _TABOO_OUT_CONFIG_OPTION(keyPrefixes)
        _TABOO_OUT_CONFIG_OPTION(keyPrefix)
        _TABOO_OUT_CONFIG_OPTION(keyFilters)
        _TABOO_OUT_CONFIG_OPTION(keyExcludes)
        _TABOO_OUT_CONFIG_OPTION(keyFields)
        _TABOO_OUT_CONFIG_OPTION(keyNum)

        _TABOO_OUT_CONFIG_OPTION(prefixMinLen)
        _TABOO_OUT_CONFIG_OPTION(prefixMaxLen)
    );
}

template<typename T> T Config::to(const std::string& name) const
{
    try {
        return options[name].as<T>();
    } catch (const std::exception& e) {
        throw std::logic_error("bad config value for '" + name + "'"
            + (noFile ? "" : (", config file: '" + file.string() + "'")));
    }
}

template<typename IntType> IntType Config::toInteger(const std::string& name) const
{
    const std::string value = to<std::string>(name);
    if (value.empty()) {
        throw std::logic_error("config value for '" + name + "' must not be empty");
    }
    if (*value.begin() == '-' && 0 < static_cast<IntType>(-1)) {
        throw std::logic_error("config value for '" + name
            + "' must be positive integer, but got '" + value + "'");
    }
    int unit = *value.rbegin();
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
        return boost::lexical_cast<IntType>(value);
    }
    if (bits == -1) {
        throw std::logic_error("bad config value '" + value + "' for '" + name + "'");
    }
    IntType num = boost::lexical_cast<IntType>(value.substr(0, value.length() - 1));
    int bitsRemain = sizeof(IntType) - bits;
    IntType max = 1 << bitsRemain;
    if (!(num < max)) {
        throw std::logic_error("config value for '" + name + "' out of bound: '" + value + "'");
    }
    return num << bits;
}

Config* Config::_instance = NULL;

boost::mutex Config::configLoadMutex;

}

#undef _TABOO_OUT_CONFIG_OPTION
