// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/call_then_write.c
// Pins: 0039 item 4: the result the wrong placement would give (23: the write before the call, then bump makes x 3): the postcondition fails at the return, so the order is pinned
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int comma_write(void)
    CERASTES_ENSURES(r, r == 23)
{
  int x = 1;
  int s = (x = (bump(&x), 2));
  return s * 10 + x;
}
