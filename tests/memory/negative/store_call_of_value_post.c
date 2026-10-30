// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/store_call_of_value.c
// Pins: 0039 item 2 (audit S6): the same program promising +2: the postcondition fails at the closing brace
#include <cerastes.h>
#include <stddef.h>

int bump2(int *p, int v)
    CERASTES_REQUIRES(cerastes_owns(*p) && v == *p && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) && r == v + 1)
{
  (void)p;
  return v + 1;
}

void use_bump2(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 2)
{
  *p = bump2(p, *p);
}
