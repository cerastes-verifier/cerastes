// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/bump_in_logic.c
// Pins: 0039 item 4 (audit B1): the value a read of x before the call would give (x == 0 true, result 1): the postcondition fails at the return, so the order of the read in && is pinned
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int and_after_call(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  return bump(&x) && x == 0;
}
