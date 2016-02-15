
#include "Session.hpp"
extern "C" {
#   include <limits.h>
#   include <sys/time.h>
}
#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(CHAR_BIT == 8);

namespace taboo {

SessionManager* SessionManager::_instance = new SessionManager;

SessionManager::RandGenerator SessionManager::rander(std::time(NULL), 0, LONG_MAX);

}
