// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2: the plain int *p form with a body that adds 2 where the written ensures says old(*p) + 1: the marker checks pointers only, so the value clause is an ordinary postcondition, reported at the closing brace of bump; the caller (which relies on the contract) reports nothing
// Pre-registered from record 0058 item 2: one postcondition error at the closing brace of bump
#include <cerastes.h>

void bump(int *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1)
{
  *p += 2;
}  // <- here

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  int x = 1;
  bump(&x);
  return x;
}
