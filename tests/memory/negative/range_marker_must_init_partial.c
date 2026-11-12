// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/range_marker_must_init.c
// Pins: 0060 item 12 (S44 (h) filler_fail): the filler leaves p[2] unwritten: it returns without initializing every element of p[0..3), which must_init promises: postcondition at the closing brace (the caller is fine: it trusts the contract)
// Pre-registered from record 0060 item 12 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void fill3(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 3))
    CERASTES_ENSURES(cerastes_owns_range(p, 3) && p[0] == 1)
{
  p[0] = 1;
  p[1] = 2;
}

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  CERASTES_UNINIT int a[3];
  fill3(a);
  return a[0] + (a[2] - a[2]);
}
