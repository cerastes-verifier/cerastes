// expect: error out-of-bounds @ L13
// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/array_param_owns_range.c
// Pins: 0060 item 15 (S43 N1): a[3] is one past owns_range(a, 3): the bound row and the slot's permission each fail at the read
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int first(int a[3])
    CERASTES_REQUIRES(cerastes_owns_range(a, 3))
    CERASTES_ENSURES(r, cerastes_owns_range(a, 3) && r == a[0] && a[0] == cerastes_old(a[0]))
{
  return a[3];
}

int use(void)
    CERASTES_ENSURES(r, r == 7)
{
  int x[3] = {7, 8, 9};
  return first(x);
}
