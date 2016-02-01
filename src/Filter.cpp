
#include "Filter.hpp"
#include "stage/misc.hpp"

namespace taboo
{

const Value FilterChain::key_in("$in", CS_CONST_STRLEN("$in"));
const Value FilterChain::key_range("$between", CS_CONST_STRLEN("$between"));
const Value FilterChain::key_attr("$follow", CS_CONST_STRLEN("$follow"));
const Value FilterChain::key_exclude("$exclude", CS_CONST_STRLEN("$exclude"));

}
