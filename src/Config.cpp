
#include "Config.hpp"
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include "stage/sys.hpp"
#include "stage/net.hpp"

#define _CSOCKS_OUT_CONFIG_PROPERTY(property)     << CS_OC_GREEN(#property) << ":\t\t" << CS_OC_RED(property) << std::endl

namespace taboo
{
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
    desc.add_options()
        ("host", boost::program_options::value<std::string>()->default_value(stage::getLanIP()))

        ("port", boost::program_options::value<uint16_t>()->default_value(1080))

        ("pid-file", boost::program_options::value<boost::filesystem::path>()->default_value(defaultPidFile),
            ("pid file, default" + defaultPidFile.string() + ".").c_str())

        ("reuse-address", boost::program_options::bool_switch()->default_value(true),
            "whether reuse-address on startup or not, default on.")

        ("tcp-nodelay", boost::program_options::bool_switch()->default_value(true),
            "enables tcp-nodelay feature or not, default on.")

        ("memlock", boost::program_options::bool_switch()->default_value(false))

        ("stack-size", boost::program_options::value<std::size_t>()->default_value(stage::getRlimitCur(RLIMIT_STACK)),
            "stack size limit (KB), default not set.")

        ("worker-count", boost::program_options::value<std::size_t>()->default_value(stage::getCpuNum() - 1),
            "num of worker processed, default is num of CPUs minus 1.")

        ("io-threads", boost::program_options::value<std::size_t>()->default_value(1),
            "num of io threads, default 1.")

        ("max-connections", boost::program_options::value<std::size_t>()->default_value(100000),
            "max in and out coming connections, default 100000.")

        ("listen-backlog", boost::program_options::value<std::size_t>()->default_value(1024),
            "listen backlog, default 1024.")

        ("max-open-files", boost::program_options::value<std::size_t>()->default_value(stage::getRlimitCur(RLIMIT_NOFILE)),
            "max open files, default not set.")

        ("downstream-receive-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "timeout for receive from downstream (in second), 0 stands for never timeout, default 30s.")

        ("downstream-send-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "timeout for send to downstream (in second), 0 stands for never timeout, default 30s.")

        ("upstream-receive-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "timeout for receive from uptream (in second), 0 stands for never timeout, default 30s.")

        ("upstream-send-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "timeout for send to uptream (in second), 0 stands for never timeout, default 30s.")

        ("initial-buffer-size", boost::program_options::value<std::size_t>()->default_value((2)),
            "size of send buffer (KB), default is 2KB.")

        ("max-matches", boost::program_options::value<std::size_t>()->default_value((100)),
            "max match count to repsonse, default is 100.")

        ("default-matches", boost::program_options::value<std::size_t>()->default_value((10)),
            "default match count to repsonse, default is 10.")
    ;
}

void Config::load(boost::filesystem::path file)
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

    host = boost::asio::ip::address_v4::from_string(options["host"].as<std::string>());
    port = options["port"].as<uint16_t>();
    pidFile = options["pid-file"].as<boost::filesystem::path>();
    reuseAddress = options["reuse-address"].as<bool>();
    tcpNodelay = options["tcp-nodelay"].as<bool>();
    memlock = options["memlock"].as<bool>();
    stackSize = options["stack-size"].as<std::size_t>() << 10;
    workerCount = options["worker-count"].as<std::size_t>();
    ioThreads = options["io-threads"].as<std::size_t>();
    maxConnections = options["max-connections"].as<std::size_t>();
    backlog = options["listen-backlog"].as<std::size_t>();
    maxOpenFiles = options["max-open-files"].as<std::size_t>();

    initBufferSize = options["initial-buffer-size"].as<std::size_t>() << 10;

    dsRecvTimeout = options["downstream-receive-timeout"].as<std::time_t>();
    dsSendTimeout = options["downstream-send-timeout"].as<std::time_t>();
    usRecvTimeout = options["upstream-receive-timeout"].as<std::time_t>();
    usSendTimeout = options["upstream-send-timeout"].as<std::time_t>();

    max_matches = options["max-matches"].as<std::size_t>();
    default_matches = options["default-matches"].as<std::size_t>();

    multiThreads = workerCount > 1;
    multiIoThreads = ioThreads > 1;

    CS_SAY(
        "loaded configs in [" << file.string() << "]:" << std::endl
        _CSOCKS_OUT_CONFIG_PROPERTY(programName)
        _CSOCKS_OUT_CONFIG_PROPERTY(host)
        _CSOCKS_OUT_CONFIG_PROPERTY(port)
        _CSOCKS_OUT_CONFIG_PROPERTY(pidFile)
        _CSOCKS_OUT_CONFIG_PROPERTY(workerCount)
        _CSOCKS_OUT_CONFIG_PROPERTY(ioThreads)
        _CSOCKS_OUT_CONFIG_PROPERTY(stackSize)
        _CSOCKS_OUT_CONFIG_PROPERTY(memlock)
        _CSOCKS_OUT_CONFIG_PROPERTY(maxOpenFiles)
        _CSOCKS_OUT_CONFIG_PROPERTY(reuseAddress)
        _CSOCKS_OUT_CONFIG_PROPERTY(maxConnections)
        _CSOCKS_OUT_CONFIG_PROPERTY(backlog)
        _CSOCKS_OUT_CONFIG_PROPERTY(tcpNodelay)
        _CSOCKS_OUT_CONFIG_PROPERTY(ioServiceNum)
        _CSOCKS_OUT_CONFIG_PROPERTY(dsRecvTimeout)
        _CSOCKS_OUT_CONFIG_PROPERTY(dsSendTimeout)
        _CSOCKS_OUT_CONFIG_PROPERTY(usRecvTimeout)
        _CSOCKS_OUT_CONFIG_PROPERTY(usSendTimeout)
        _CSOCKS_OUT_CONFIG_PROPERTY(initBufferSize)
    );
}

Config Config::_instance;

}

#undef _CSOCKS_OUT_CONFIG_PROPERTY
