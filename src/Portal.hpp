
#include "predef.hpp"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <glog/logging.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "Config.hpp"
#include "Aside.hpp"
#include "Keeper.hpp"
#include "Seeker.hpp"
#include "Manager.hpp"

namespace taboo  {

class Portal:
    public boost::enable_shared_from_this<Portal>
{
private:
    int argc;
    char** argv;
public:
    Portal(int _argc, char** _argv):
        argc(_argc), argv(_argv)
    {}

    void startHttp() const
    {
        taboo::Manager::instance()->start();
    }

    static void onMessage(Server *server, websocketpp::connection_hdl hdl,
        Server::message_ptr message)
    {
        SharedReply reply = Router::instance()->route(message)->process();
        server->send(hdl, reply->content, message->get_opcode());
    }

    void startWs() const
    {
        Server server;
        server.set_message_handler(websocketpp::lib::bind(
            &Portal::onMessage, &server,
            websocketpp::lib::placeholders::_1,
            websocketpp::lib::placeholders::_2));

        {
            server.init_asio();
            boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string(
                Config::instance()->queryHost), Config::instance()->queryPort);
            server.listen(ep);
            server.start_accept();
        }

        server.run();
    }

    bool init() const
    {
        google::InitGoogleLogging(argv[0]);
        return (!taboo::Config::initialize(argc, argv)
            || !taboo::Aside::initialize()
            || !taboo::Keeper::initialize()
            || !taboo::Manager::initialize()
            || !taboo::Router::initialize());
    }

    void start() const
    {
        boost::thread http(boost::bind(&Portal::startHttp, shared_from_this()));
        http.join();
        CS_SAY("http started");

        CS_SAY("ws starting");
        startWs();
    }
};

typedef boost::shared_ptr<Portal> SharedPortal;

}
