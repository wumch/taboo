
/**
curl -vvv http://localhost:9002     \
 -H "Connection:Upgrade" -H "Origin:null" -H "Sec-WebSocket-Extensions:x-webkit-deflate-frame"          \
 -H "Sec-WebSocket-Key:puVOuWb7rel6z2AVZBKnfw==" -H "Sec-WebSocket-Version:13" -H "Upgrade:websocket"
 */

#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "trie.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

void test_trie()
{
    taboo::Trie trie;
    taboo::Item item(10086);
    trie.attach("abcdefg", item);
    taboo::ItemIdList items = trie.match("abcdefg");
    CS_DUMP(items[0]);
    items = trie.match("abcdefg4");
    CS_DUMP(items.size());
}

void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg)
{
    std::cout << msg->get_payload() << std::endl;
}

int main()
{
    CS_SAY("building trie");
    test_trie();
    CS_SAY("build_trie done");

    server print_server;

    print_server.set_message_handler(&on_message);

    print_server.init_asio();
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 9002);
    print_server.listen(ep);
    print_server.start_accept();

    print_server.run();
}