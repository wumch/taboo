
/**
curl -vvv http://localhost:9002     \
 -H "Connection:Upgrade" -H "Origin:null" -H "Sec-WebSocket-Extensions:x-webkit-deflate-frame"          \
 -H "Sec-WebSocket-Key:puVOuWb7rel6z2AVZBKnfw==" -H "Sec-WebSocket-Version:13" -H "Upgrade:websocket"
 */

#include "predef.hpp"
#include <iostream>
#include <glog/logging.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "Config.hpp"
#include "Aside.hpp"
#include "Keeper.hpp"
#include "Seeker.hpp"
#include "Manager.hpp"

namespace taboo
{

void test_trie(const char* query)
{
    {
        KeyList keys;
        keys.push_back("abcdefg");
        keys.push_back("abcdefghi");
        keys.push_back("abdef");
        SharedItem item = makeItem("{\"id\":10086,\"name\":\"wumch\",\"age\":45}");
        Keeper::instance()->attach(keys, item, true);
    }

    {
        KeyList keys;
        keys.push_back("abcdrg");
        keys.push_back("abcdef454i");
        keys.push_back("abdef白入定");
        SharedItem item = makeItem("{\"id\":10087,\"name\":\"入定\",\"age\":38}");
        Keeper::instance()->attach(keys, item, true);
    }
}

}

typedef websocketpp::server<websocketpp::config::asio> server;

void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg)
{
    std::cout << msg->get_payload() << std::endl;
}

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    if (!taboo::Config::initialize(argc, argv)
        || !taboo::Aside::initialize()
        || !taboo::Keeper::initialize()
        || !taboo::Manager::initialize()
        || !taboo::Router::initialize()) {
        CS_DIE("failed on initialize");
    }

    CS_SAY("testing trie");
    taboo::test_trie(argv[argc - 1]);
    CS_SAY("test trie done");

    taboo::Manager::instance()->start();
    getchar();

    return 0;

    server print_server;

    print_server.set_message_handler(&on_message);

    print_server.init_asio();
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 9002);
    print_server.listen(ep);
    print_server.start_accept();

    print_server.run();
}
