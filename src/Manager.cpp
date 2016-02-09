
#include "Manager.hpp"

namespace taboo
{

Manager* Manager::_instance = NULL;

Keeper Manager::keeper;

Manager::ErrMap Manager::errs;

std::string Manager::okResponse;

boost::mutex Manager::initMutex;

}
