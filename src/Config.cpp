
#include "Config.hpp"
extern "C" {
#   include <limits.h>
}
#include <algorithm>
#include <iostream>
#include <string>
#include <exception>
#include <memory>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include "stage/sys.hpp"
#include "stage/net.hpp"
#include "stage/iostream.hpp"

#define _TABOO_OUT_CONFIG_OPTION(property)     << CS_OC_GREEN(#property)         \
    << ":\t\t" << CS_OC_RED(property) << std::endl

namespace taboo {

namespace {

class ErrorMissOption:
    public po::error_with_option_name
{
public:
    ErrorMissOption(const std::string& optionName, const std::string& reason = std::string()):
        po::error_with_option_name("config option '%option%' is required"
            + (reason.empty() ? reason : (": " + reason)), optionName)
    {
        set_substitute("option", optionName);
    }
};

class ErrorInvalidValue:
    public po::error_with_option_name
{
public:
    ErrorInvalidValue(const std::string& optionName, const std::string& optionValue = "",
        const std::string& reason = std::string()):
        po::error_with_option_name("invalid config value "
            + (optionValue.empty() ? "" : ("'" + optionValue + "' ")) + "for option '%option%'"
            + (reason.empty() ? reason : (": " + reason)), optionName)
    {
        set_substitute("option", optionName);
    }
};

class ErrorBadValue:
    public po::error_with_option_name
{
public:
    ErrorBadValue(const std::string& optionValue, const std::string& reason = std::string()):
        po::error_with_option_name("invalid config value "
            + (optionValue.empty() ? "" : ("'" + optionValue + "' ")) + "for option '%option%'"
            + (reason.empty() ? reason : (": " + reason)))
    {}
};

}

template<typename IntType> IntType toInteger(const std::string& value)
{
    std::string name("config-option-name-aaa");
    if (value.empty()) {
        throw ErrorBadValue("", "must not be empty");
    }
    if (*value.begin() == '-' && 0 < static_cast<IntType>(-1)) {
        throw ErrorBadValue(value, "must be positive integer");
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

    IntType num;
    try {
        num = boost::lexical_cast<IntType>(value.substr(0, value.length() - 1));
    } catch (const boost::bad_lexical_cast& e) {
        throw ErrorBadValue(value, "quantity part must be integer");
    }

    if (bits == -1) {
        throw ErrorBadValue(value, std::string("bad unit '") + (char)unit + "'");
    }
    if (bits) {
        int bitsRemain = sizeof(IntType) * CHAR_BIT - bits;
        if (bitsRemain < 0) {
            throw ErrorBadValue(value, std::string("unit '") + (char)unit + "' out of bound");
        }
        IntType max = 1 << bitsRemain;
        if (!(num < max)) {
            throw ErrorBadValue(value, "out of bound");
        }
        return num << bits;
    } else {
        return num;
    }
}

template<typename IntType>
class SizeType
{
public:
    IntType value;

    SizeType(IntType _value):
        value(_value)
    {}

    SizeType(const std::string& strValue):
        value(toInteger<IntType>(strValue))
    {}

    operator IntType() const
    {
        return value;
    }
};

template<typename IntType>
SizeType<IntType>* makePtr(IntType& i)
{
    return reinterpret_cast<SizeType<IntType>*>(&i);
}

template<typename IntType>
void validate(boost::any& res, const std::vector<std::string>& values, SizeType<IntType>* target, int)
{
    po::validators::check_first_occurrence(res);
    const std::string& origin = po::validators::get_single_string(values);
    res = boost::any(SizeType<IntType>(toInteger<IntType>(origin)));
}

bool Config::initialize(int argc, char* argv[])
{
    typedef std::auto_ptr<Config> ConfigPtr;
    {
        try {
            boost::mutex::scoped_lock lock(configLoadMutex);
            ConfigPtr config(new Config);
            config->init(argc, argv);
            _instance = config.release();
        } catch (const ErrorMissOption& e) {
            CS_DIE("error: " << e.what());
        } catch (const ErrorInvalidValue& e) {
            CS_DIE("error: " << e.what());
        } catch (const po::error& e) {
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

    defaultConfigDir = "etc";
    boost::filesystem::path defaultConfig = defaultConfigDir / (programName + ".conf");
    desc.add_options()
        ("help,h", "show this help and exit.")
        ("test-config,t", "test config options and exit.")
        ("reload-config,r", "reload config options from previous loaded config file. "
            "(NOTE: new specified command line options are omitted, and "
            "old command line options will be overridden by ones from new config file)")
        ("config,c", po::value(&configFile)->default_value(defaultConfig),
            ("config file, default is " + defaultConfig.string() + ". "
                "options from config file will be overridden by ones from command line.").c_str())
        ("no-config-file", po::bool_switch()->default_value(false),
            "force do not load options from config file, default false.");
    initDesc();

    try {
        po::command_line_parser parser(argc, argv);
        parser.options(desc).style(po::command_line_style::unix_style);
        po::store(parser.run(), options);
    } catch (const std::exception& e) {
        throw po::error(std::string(e.what()) + CS_LINESEP
            + CS_OC_BLACK_BEGIN + boost::lexical_cast<std::string>(desc) + CS_OC_END);
    }
    po::notify(options);

    if (options.count("manage-key") || options.count("manage-secret")) {
        CS_ERR("warning: specify 'manage-key' or 'manage-secret' in command line is dangerous.");
    }

    purposeShowHelp = options.count("help");
    purposeTestConfig = options.count("test-config");
    purposeReloadConfig = options.count("reload-config");

    if (purposeShowHelp) {
        std::cout
            << programName << " is an HTTP/WebSocket based prefix predict server with "
                "user-customizeable property " << CS_LINESEP
                << "matching feature. See " << TABOO_WIKI_LINK << " for more information."
            << CS_LINESEP << CS_LINESEP
            << "usage: " << argv[0] << " [options]" << CS_LINESEP
            << desc << CS_LINESEP;
    } else {
        noFile = to<bool>("no-config-file");
        if (noFile) {
            loadOptions();
        } else {
            configFile = to<boost::filesystem::path>("config");
            loadFile();
        }
    }

    if (purposeTestConfig) {
        CS_ECHO(CS_OC_GREEN("config " << (configFile.empty() ? "" :
            (std::string("file '") + configFile.string() + "'")) << " is ok."));
    }
}

void Config::initDesc()
{
    boost::filesystem::path defaultPidFile("/var/run/" + programName + ".pid");
    boost::filesystem::path defaultStorePath("/var/lib/" + programName + "/");
    boost::filesystem::path defaultTrieFile = defaultStorePath / "trie.dat",
        defaultItemsFile = defaultStorePath / "items.dat";
    boost::filesystem::path defaultWssCert = defaultConfigDir / "wss.cert",
        defaultHttpsCert = defaultConfigDir / "https.cert";
    std::string lanIp = stage::getLanIP();
    desc.add_options()
        ("pid-file", po::value(&pidFile)->default_value(defaultPidFile),
            ("pid file, default" + defaultPidFile.string() + ".").c_str())

        ("store-on-exit", po::bool_switch(&storeOnExit)->default_value(true),
            "store trie/items into @trie-file/@items-file before exit or not, default is yes.")
        ("restore-on-start", po::bool_switch(&restoreOnStart)->default_value(true),
            "restore trie/items from @trie-file/@items-file when startup or not, default is yes.")

        ("trie-file", po::value(&trieFile)->default_value(defaultTrieFile),
            ("file to store trie, default is '" + defaultStorePath.string() + "trie.dat'.").c_str())
        ("items-file", po::value(&itemsFile)->default_value(defaultItemsFile),
            ("file to store items, default is '" + defaultStorePath.string() + "items.dat'.").c_str())

        ("query-enable-http", po::bool_switch(&queryEnableHttp)->default_value(true),
            "enable HTTP protocol for query, default is 'yes'.")
        ("query-enable-https", po::bool_switch(&queryEnableHttps)->default_value(false),
            "enable HTTPS protocol for query, default is 'false'.")
        ("query-enable-websocket", po::bool_switch(&queryEnableWs)->default_value(true),
            "enable WebSocket protocol for query, default is 'yes'.")
        ("query-enable-websocket-secure", po::bool_switch(&queryEnableWss)->default_value(false),
            "enable WebSocket Secure protocol for query, default is 'yes'.")
        ("https-cert", po::value(&httpsCert)->default_value(defaultHttpsCert),
            ("cert file for HTTPS protocol, default is '" + defaultHttpsCert.string() + "'.").c_str())
        ("websocket-secure-cert", po::value(&wssCert)->default_value(defaultWssCert),
            ("cert file for WebSocket Secure protocol, default is '"
            + defaultWssCert.string() + "'.").c_str())

        ("manage-host", po::value(&manageHost)->default_value(lanIp),
            ("host to bind for manage, default is " + lanIp).c_str())
        ("manage-port", po::value(&managePort)->default_value(1079),
            "port to bind for manage, default is 1079")
        ("query-host", po::value(&queryHost)->default_value(lanIp),
            ("host to bind for query, default is " + lanIp).c_str())
        ("query-port", po::value(&queryPort)->default_value(1080),
            "port to bind for query, default is 1080")

        ("manage-workers", po::value(makePtr(manageWorkers))->default_value(1),
            "num of workers for manage, default is 1.")
        ("query-workers", po::value(makePtr(queryWorkers))->default_value(
            std::min<std::size_t>(1, stage::getCpuNum() - 1)),
            "num of workers for query, default is num of CPU cores minus 1.")

        ("stack-size", po::value(makePtr(stackSize))->default_value(0),
            "stack size limit, 0 is not set, default is 0.")
        ("max-open-files", po::value(makePtr(maxOpenFiles))->default_value(0),
            "max open files, 0 is not set, default is 0.")
        ("memlock", po::bool_switch(&memlock)->default_value(false),
            "memlock after startup or not, default is no")
        ("reuse-address", po::bool_switch(&reuseAddress)->default_value(true),
            "whether reuse-address on startup or not, default on.")
        ("tcp-nodelay", po::bool_switch(&tcpNodelay)->default_value(true),
            "enables tcp-nodelay feature or not, default on.")
        ("listen-backlog", po::value(makePtr(backlog))->default_value(std::string("1K")),
            "listen backlog, default is 1K.")

        ("max-manage-connections", po::value(makePtr(maxManageConnections))->default_value(std::string("10K")),
            "max manage connections, default 10K.")
        ("max-query-connections", po::value(makePtr(maxQueryConnections))->default_value(std::string("1M")),
            "max query connections, default 1M.")

        ("manage-connection-memory-limit", po::value(makePtr(manageConnectionMemoryLimit))->default_value(std::string("256K")),
            "memory size limit per manage connection, 0 is unlimited, default is 256K.")
        ("manage-connection-recv-buffer", po::value(makePtr(manageRecvBuffer))->default_value(std::string("128K")),
            "recv buffer size for manage connections, default is 128K.")
        ("manage-connection-send-buffer", po::value(makePtr(manageSendBuffer))->default_value(std::string("128K")),
            "send buffer size for manage connections, default is 128K.")
        ("query-connection-recv-buffer", po::value(makePtr(queryRecvBuffer))->default_value(std::string("128K")),
            "send buffer size for query connections, default is 128K.")
        ("query-connection-send-buffer", po::value(makePtr(querySendBuffer))->default_value(std::string("4K")),
            "send buffer size for query connections, default is 4K.")

        ("manage-receive-timeout", po::value(&manageRecvTimeout)->default_value(30),
            "receive-timeout for manage requests (second), 0 is unlimited, default is 30s.")
        ("manage-send-timeout", po::value(&manageSendTimeout)->default_value(30),
            "send-timeout manage requests (second), 0 is unlimited, default is 30s.")
        ("query-receive-timeout", po::value(&queryRecvTimeout)->default_value(30),
            "receive-timeout for query requests (second), 0 for unlimited, default is 30s.")
        ("query-send-timeout", po::value(&querySendTimeout)->default_value(30),
            "send-timeout for query requests (second), 0 is unlimited, default is 30s.")

        ("connection-max-idle", po::value(&itemsAllocStep)->default_value(7200),
            "max idle time for each connection (second), default is 7200.")
        ("connection-check-interval", po::value(&connectionCheckInterval)->default_value(600),
            "connection idle time check interval (second), default is 600.")

        ("items-allocate-step", po::value(makePtr(itemsAllocStep))->default_value(std::string("10K")),
            "count of items to allocate for each increment, default is 10K.")
        ("max-items", po::value(makePtr(maxItems))->default_value(std::string("0")),
            "max count of items, 0 for unlimited, default is 0.")

        ("prefix-min-length", po::value(makePtr(prefixMinLen))->default_value(2),
            "min length for prefix of query, at least 1, default is 2.")
        ("prefix-max-length", po::value(makePtr(prefixMaxLen))->default_value(60),
            "max length for prefix of query, default is 60. NOTE: this is not applied for manage requests.")
        ("query-data-max-bytes", po::value(makePtr(queryDataMaxSize))->default_value(std::string("4K")),
            "max bytes of @key-query-request-payload for query requests, default is 4K.")

        ("max-iterations", po::value(makePtr(maxIterations))->default_value(std::string("10K")),
            "max iterations for each matching, default is 10K.")
        ("max-matches", po::value(makePtr(maxMatches))->default_value(std::string("100")),
            "max count of items to match for query, default is 100.")
        ("default-matches", po::value(makePtr(defaultMatches))->default_value(std::string("10")),
            "default count of items to match for query, default is 10.")

        ("check-signature", po::bool_switch(&checkSign)->default_value(true),
            "check signature or not for manage requests, default is yes.")
        ("manage-must-post", po::bool_switch(&manageMustPost)->default_value(false),
            "force request method of manage requests to be POST, default is 'no'.")
        ("manage-key", po::value(&manageKey),
            "access key for manage, REQUIRED when @check-signature='yes'.")
        ("manage-secret", po::value(&manageSecret),
            "access secret for manage, REQUIRED when @check-signature='yes'.")
        ("sign-hyphen", po::value(&signHyphen)->default_value("="),
            "glue for join key and value when generating signature, default is '='.")
        ("sign-delimiter", po::value(&signDelimiter)->default_value("&"),
            "glue for join key-value pairs when generate signature, default is '&'.")

        ("key-manage-request-manage-key", po::value(&keyMUKey)->default_value("key"),
            "key name for 'manage-key' of manage requests, default is 'key'.")
        ("key-manage-request-sign", po::value(&keyMUSign)->default_value("sign"),
            "key name for 'sign' of manage requests, default is 'sign'.")
        ("key-manage-request-prefixes", po::value(&keyMUPrefixes)->default_value("prefixes"),
            "key name for 'prefixes' of items in manage requests, default is 'prefixes'.")
        ("key-manage-request-item", po::value(&keyMUItem)->default_value("item"),
            "key name for 'item' of manage requests, default is 'item'.")
        ("key-item-id", po::value(&keyId)->default_value("id"),
            "key name for 'id' of items, default is 'id'.")
        ("key-manage-request-upsert-item", po::value(&keyMUUpsert)->default_value("upsert"),
            "key name for 'upsert-item' of manage requests, default is 'upsert'.")
        ("key-manage-request-token-identy", po::value(&keyMUIdenty)->default_value("tid"),
            "key name for 'identy of token' of get token requests, default is 'tid'.")
        ("key-manage-request-filters", po::value(&keyMUFilters)->default_value("filters"),
            "key name for 'filters' of get token requests, default is 'filters'.")
        ("key-manage-request-expire", po::value(&keyMUExpire)->default_value("expire"),
            "key name for 'token expire' of get token requests, default is 'expire'.")

        ("key-manage-response-error-code", po::value(&keyMUExpire)->default_value("code"),
            "key name for 'error-code' of manage responses, default is 'code'.")
        ("key-manage-response-error-description", po::value(&keyMUExpire)->default_value("desc"),
            "key name for 'error-description' of manage responses, default is 'desc'.")
        ("key-manage-response-payload", po::value(&keyMUExpire)->default_value("data"),
            "key name for 'payload' of manage responses, default is 'data'.")

        ("key-query-request-access-token", po::value(&keyQUToken)->default_value("accessToken"),
            "key name for access-token of query requests, default is 'accessToken'.")
        ("key-query-request-payload", po::value(&keyQUPayload)->default_value("data"),
            "key name for query data of query requests, default is 'data'.")
        ("key-query-request-prefix", po::value(&keyQUPrefix)->default_value("prefix"),
            "key name for 'prefix' of query requests, default is 'prefix'.")
        ("key-query-request-filters", po::value(&keyQUFilters)->default_value("filters"),
            "key name for 'filters' of query requests, default is 'filters'.")
        ("key-query-request-excludes", po::value(&keyQUExcludes)->default_value("excludes"),
            "key name for 'excludes' of query requests, default is 'excludes'.")
        ("key-query-request-fields", po::value(&keyQUFields)->default_value("fields"),
            "key name for 'fields' of query requests, default is 'fields'.")
        ("key-query-request-num", po::value(&keyQUNum)->default_value("num"),
            "key name for 'num' of query requests, default is 'num'.")
        ("key-query-request-echo-data", po::value(&keyQEchoData)->default_value("echoData"),
            "key name for 'echo-data' of both query requests and responses, default is 'echoData'. "
            "explicit specify this as empty string will disable 'echo-data' feature.")

        ("key-query-response-error-code", po::value(&keyQDErrCode)->default_value("code"),
             "key name for 'error-code' of query responses, default is 'code'.")
        ("key-query-response-error-description", po::value(&keyQDErrDesc)->default_value("desc"),
            "key name for 'error-description' of query responses, default is 'desc'.")
        ("key-query-response-payload", po::value(&keyQDPayload)->default_value("data"),
            "key name for 'payload' of query responses, default is 'data'.")

        ("query-visible-fields", po::value<std::string>()->default_value("*"),
            "visible fields for query requests, comma separated field names, "
            "default is '*', means all fields are visible.")
        ("query-invisible-fields", po::value<std::string>()->default_value(""),
            "invisible fields for query requests, comma separated field names. "
            "if 'query-visible-fields' is specified other than '*', then this one is omitted. "
            "default is '', means all fields excluding 'query-visible-fields' are invisible.")
    ;
}

void Config::loadFile()
{
    try {
        po::store(po::parse_config_file<char>(configFile.c_str(), desc), options);
    } catch (const std::exception& e) {
        boost::system::error_code err;
        boost::filesystem::path path = boost::filesystem::canonical(configFile, err);
        throw po::error("faild on load config-file: "
            + (err ? configFile : path).string() + CS_LINESEP_STR + e.what());
    }
    po::notify(options);

    loadOptions();
}

void Config::loadOptions()
{
    if (checkSign) {
        if (options.find("manage-key") == options.end()) {
            throw ErrorMissOption("manage-key");
        }
        if (options.find("manage-secret") == options.end()) {
            throw ErrorMissOption("manage-secret");
        }
        manageKey = to<std::string>("manage-key");
        if (manageKey.empty()) {
            throw ErrorInvalidValue("manage-key", "", "must not be empty");
        }
        manageSecret = to<std::string>("manage-secret");
        if (manageSecret.empty()) {
            throw ErrorInvalidValue("manage-secret", "", "must not be empty");
        }
    }

    queryVisibleFields = series<std::string>("query-visible-fields");
    bool visibleAll = queryVisibleFields.size() == 1 && queryVisibleFields[0] == "*";
    if (queryVisibleFields.empty() || visibleAll) {
        queryInvisibleFields = series<std::string>("query-invisible-fields");
    }
    queryVisibleAll = visibleAll && queryInvisibleFields.empty();

#if CS_DEBUG
    boost::system::error_code err;
    boost::filesystem::path path = boost::filesystem::canonical(configFile, err);
#endif
    CS_SAY(
        "loaded configs in [" << (err ? configFile : path) << "]:" << CS_LINESEP

        _TABOO_OUT_CONFIG_OPTION(programName)
        _TABOO_OUT_CONFIG_OPTION(configFile)
        _TABOO_OUT_CONFIG_OPTION(pidFile)

        _TABOO_OUT_CONFIG_OPTION(storeOnExit)
        _TABOO_OUT_CONFIG_OPTION(restoreOnStart)
        _TABOO_OUT_CONFIG_OPTION(trieFile)
        _TABOO_OUT_CONFIG_OPTION(itemsFile)

        _TABOO_OUT_CONFIG_OPTION(queryEnableHttp)
        _TABOO_OUT_CONFIG_OPTION(queryEnableWs)
        _TABOO_OUT_CONFIG_OPTION(queryEnableWss)
        _TABOO_OUT_CONFIG_OPTION(wssCert)
        _TABOO_OUT_CONFIG_OPTION(httpsCert)

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

        _TABOO_OUT_CONFIG_OPTION(maxManageConnections)
        _TABOO_OUT_CONFIG_OPTION(maxQueryConnections)

        _TABOO_OUT_CONFIG_OPTION(manageConnectionMemoryLimit)
        _TABOO_OUT_CONFIG_OPTION(manageRecvBuffer)
        _TABOO_OUT_CONFIG_OPTION(manageSendBuffer)
        _TABOO_OUT_CONFIG_OPTION(queryRecvBuffer)
        _TABOO_OUT_CONFIG_OPTION(querySendBuffer)

        _TABOO_OUT_CONFIG_OPTION(queryRecvTimeout)
        _TABOO_OUT_CONFIG_OPTION(querySendTimeout)
        _TABOO_OUT_CONFIG_OPTION(manageRecvTimeout)
        _TABOO_OUT_CONFIG_OPTION(manageSendTimeout)

        _TABOO_OUT_CONFIG_OPTION(connectionMaxIdle)
        _TABOO_OUT_CONFIG_OPTION(connectionCheckInterval)

        _TABOO_OUT_CONFIG_OPTION(itemsAllocStep)
        _TABOO_OUT_CONFIG_OPTION(maxItems)

        _TABOO_OUT_CONFIG_OPTION(prefixMinLen)
        _TABOO_OUT_CONFIG_OPTION(prefixMaxLen)
        _TABOO_OUT_CONFIG_OPTION(queryDataMaxSize)

        _TABOO_OUT_CONFIG_OPTION(maxIterations)
        _TABOO_OUT_CONFIG_OPTION(maxMatches)
        _TABOO_OUT_CONFIG_OPTION(defaultMatches)

        _TABOO_OUT_CONFIG_OPTION(checkSign)
        _TABOO_OUT_CONFIG_OPTION(manageKey)
        _TABOO_OUT_CONFIG_OPTION(manageSecret)
        _TABOO_OUT_CONFIG_OPTION(signHyphen)
        _TABOO_OUT_CONFIG_OPTION(signDelimiter)

        _TABOO_OUT_CONFIG_OPTION(keyMUKey)
        _TABOO_OUT_CONFIG_OPTION(keyMUSign)
        _TABOO_OUT_CONFIG_OPTION(keyMUPrefixes)
        _TABOO_OUT_CONFIG_OPTION(keyMUItem)
        _TABOO_OUT_CONFIG_OPTION(keyMUUpsert)
        _TABOO_OUT_CONFIG_OPTION(keyMUIdenty)
        _TABOO_OUT_CONFIG_OPTION(keyMUFilters)
        _TABOO_OUT_CONFIG_OPTION(keyMUExpire)

        _TABOO_OUT_CONFIG_OPTION(keyMDErrCode)
        _TABOO_OUT_CONFIG_OPTION(keyMDErrDesc)
        _TABOO_OUT_CONFIG_OPTION(keyMDPayload)

        _TABOO_OUT_CONFIG_OPTION(keyId)

        _TABOO_OUT_CONFIG_OPTION(keyQEchoData)

        _TABOO_OUT_CONFIG_OPTION(keyQUPayload)
        _TABOO_OUT_CONFIG_OPTION(keyQUToken)
        _TABOO_OUT_CONFIG_OPTION(keyQUPrefix)
        _TABOO_OUT_CONFIG_OPTION(keyQUFilters)
        _TABOO_OUT_CONFIG_OPTION(keyQUExcludes)
        _TABOO_OUT_CONFIG_OPTION(keyQUFields)
        _TABOO_OUT_CONFIG_OPTION(keyQUNum)

        _TABOO_OUT_CONFIG_OPTION(keyQDErrCode)
        _TABOO_OUT_CONFIG_OPTION(keyQDErrDesc)
        _TABOO_OUT_CONFIG_OPTION(keyQDPayload)

        _TABOO_OUT_CONFIG_OPTION(queryVisibleFields)
        _TABOO_OUT_CONFIG_OPTION(queryInvisibleFields)
        _TABOO_OUT_CONFIG_OPTION(queryVisibleAll)
    );
}

template<typename T> T Config::to(const std::string& name) const
{
    po::variables_map::const_iterator it = options.find(name);
    if (it == options.end()) {
        throw ErrorMissOption(name);
    }
    try {
        return it->second.as<T>();
    } catch (const std::exception& e) {
        throw ErrorInvalidValue(name, it->second.as<std::string>());
    }
}

template<typename T> std::vector<T> Config::series(const std::string& name) const
{
    std::string origin = options[name].as<std::string>();
    try {
        StringList segments;
        boost::split(segments, origin, boost::is_any_of(",\n"), boost::token_compress_on);
        std::vector<T> res;
        res.reserve(segments.size());
        for (StringList::iterator it = segments.begin(); it != segments.end(); ++it) {
            res.push_back(boost::lexical_cast<T>(boost::algorithm::trim_copy(*it)));
        }
        return res;
    } catch (const std::exception& e) {
        throw ErrorInvalidValue(name, origin);
    }
}

Config* Config::_instance = NULL;

boost::mutex Config::configLoadMutex;

}

#undef _TABOO_OUT_CONFIG_OPTION
