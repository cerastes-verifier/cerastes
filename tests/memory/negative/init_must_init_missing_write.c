// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 5: a path of a must_init function that does not write *p returns without initializing it: postcondition at the return
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT, int c)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  if (c != 0) {
    *p = 0;
  }
}  // <- here
