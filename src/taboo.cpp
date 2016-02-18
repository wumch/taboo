
/**
curl -vvv http://localhost:9002     \
 -H "Connection:Upgrade" -H "Origin:null" -H "Sec-WebSocket-Extensions:x-webkit-deflate-frame"          \
 -H "Sec-WebSocket-Key:puVOuWb7rel6z2AVZBKnfw==" -H "Sec-WebSocket-Version:13" -H "Upgrade:websocket"
 */

#include "Portal.hpp"

int main(int argc, char* argv[])
{
    taboo::SharedPortal portal(new taboo::Portal(argc, argv));
    portal->init();
    portal->start();
}
