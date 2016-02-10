
#include "Manager.hpp"

namespace taboo
{

Manager* Manager::_instance = NULL;

boost::mutex Manager::initMutex;

}
