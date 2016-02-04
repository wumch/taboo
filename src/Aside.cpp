
#include "Aside.hpp"
#include "stage/sys.hpp"
#include "Config.hpp"

namespace taboo
{

Aside* Aside::_instance = NULL;

bool Aside::_initialize()
{
    const Config* config = Config::instance();
    if (config->stackSize) {
        stage::setRlimit(RLIMIT_STACK, config->stackSize);
    }
    if (config->maxOpenFiles) {
        stage::setRlimit(RLIMIT_NOFILE, config->maxOpenFiles);
    }
    return true;
}

}
