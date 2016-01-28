
/**
curl -vvv http://localhost:9002     \
 -H "Connection:Upgrade" -H "Origin:null" -H "Sec-WebSocket-Extensions:x-webkit-deflate-frame"          \
 -H "Sec-WebSocket-Key:puVOuWb7rel6z2AVZBKnfw==" -H "Sec-WebSocket-Version:13" -H "Upgrade:websocket"
 */

#include "predef.hpp"
#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "cedar/cedar.h"

typedef websocketpp::server<websocketpp::config::asio> server;

void test_trie()
{
    typedef cedar::da<int32_t, -1, -2, false> Trie;
    Trie trie;
    trie.update("abcdefg", 7, 10086);
    CS_DUMP(trie.update("abcdefg", 7));
    CS_DUMP(trie.update("abcdefg", 7, 11111));
    trie.update("abcde", 5, 10087);
    trie.update("abcdefghj", 9, 10088);
    trie.update("abcde76", 7, 10089);

    int32_t res[8];
    size_t len = trie.commonPrefixPredict("abc", res, 5);
    CS_DUMP(len);
    CS_DUMP(res[0]);
    CS_DUMP(res[1]);
    CS_DUMP(res[2]);
    CS_DUMP(res[3]);
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
    return 0;

    server print_server;

    print_server.set_message_handler(&on_message);

    print_server.init_asio();
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 9002);
    print_server.listen(ep);
    print_server.start_accept();

    print_server.run();
}