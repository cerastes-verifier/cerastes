// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/memory/call_after_test.c
// Pins: 0039 item 4: the value a read of x after the call would give (always 1, as bump made x nonzero): the postcondition fails at v == 0, so the snapshot is pinned
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int and_call(int v)
    CERASTES_REQUIRES(0 <= v && v < 100)
    CERASTES_ENSURES(r, r == 1)
{
  int x = v;
  int s = x && bump(&x);
  return s;
}
