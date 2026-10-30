// expect: error postcondition @ L12
// mode: c++20
// twin: tests/memory/result_value_cpp.cpp
// Pins: 0040 item 4: *r is old(*p), not old(*p) + 1
#include <cerastes.h>
#include <stddef.h>

int *pick(int *p)
    CERASTES_REQUIRES(cerastes::owns(*p) && *p < 2147483647)
    CERASTES_ENSURES(r, cerastes::owns(*p) && r == p && *r == cerastes::old(*p) + 1)
{
  return p;
}
