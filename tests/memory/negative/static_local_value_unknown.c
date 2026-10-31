// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/memory/static_local_counter.c
// Pins: 0041 item 12: count is not 0 on a later entry (an earlier call may have changed it): the postcondition r == 0 cannot hold, although the initializer is 0
#include <cerastes.h>

int zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  static int count = 0;
  return count;
}
