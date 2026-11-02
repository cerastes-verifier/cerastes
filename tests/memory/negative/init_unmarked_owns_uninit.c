// expect: error uninitialized @ L14
// mode: c23, c++20
// twin: tests/memory/init_unmarked_flag_row.c
// Pins: 0042 item 7 (question 82): the flag row of a call is `uninitialized`, not `permission`: an owns parameter given an object without a value, reported at the argument's call
#include <cerastes.h>

void set(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 1);

int use_set(void)
{
  int x;
  set(&x);  // <- here
  return 0;
}
