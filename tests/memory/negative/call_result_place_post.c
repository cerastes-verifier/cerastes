// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/call_result_place.c
// Pins: 0039 item 4 (audit S2): the same program with the sum written as 6: x ends 4 and v is 1, so the postcondition fails at the return
#include <cerastes.h>
#include <stddef.h>

int *first(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, r == p && cerastes_owns(*p) && *p == cerastes_old(*p))
{
  return p;
}

int use_first(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 1;
  int v = *first(&x);
  *first(&x) += 3;
  return x + v;
}
