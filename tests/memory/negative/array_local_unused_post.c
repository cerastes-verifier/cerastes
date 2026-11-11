// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/memory/array_local_unused.c
// Pins: 0012 item 12 and 0059 (Flip A's twin): the unused array changes nothing, so r == 1 fails at the return (a is any int)
#include <cerastes.h>

int first(int a)
    CERASTES_ENSURES(r, r == 1)
{
  [[maybe_unused]] int values[3] = {1, 2, 3};
  return a;
}
