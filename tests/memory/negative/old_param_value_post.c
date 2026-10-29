// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/old_param_value.c
// Pins: 0037 item 5: the entry value of x is x, so r == x fails for x + 1
#include <cerastes.h>
#include <stddef.h>

#include <limits.h>
int succ(int x)
    CERASTES_REQUIRES(x < INT_MAX)
    CERASTES_ENSURES(r, r == cerastes_old(x))
{
  return x + 1;
}
