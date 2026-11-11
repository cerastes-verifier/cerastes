// expect: error out-of-bounds @ L20
// mode: c23, c++20
// twin: tests/memory/array_addr_symbolic.c
// Pins: 0059 item 4: forming &a[i] with i == 4 is more than one past the end (value position allows 0 <= i <= 3): out-of-bounds at the address
// Pre-registered from record 0059 item 4
#include <cerastes.h>

void set(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 5)
{
  *p = 5;
}

int use_at(int i)
    CERASTES_REQUIRES((0 <= i && i < 3) || i == 4)
    CERASTES_ENSURES(r, r == 5)
{
  int a[3] = {1, 2, 3};
  set(&a[i]);
  return a[i];
}
