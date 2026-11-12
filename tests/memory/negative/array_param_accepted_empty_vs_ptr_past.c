// expect: error out-of-bounds @ L15
// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/array_param_accepted_empty_vs_ptr.c
// Pins: 0060 item 15 (S43 N1): owns(*a) holds one element: a[1] fails its bound and its permission at the read
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int f(int a[]);

int f(int *a)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, cerastes_owns(*a))
{
  return a[1];
}

int use(void)
{
  int x[2] = {5, 6};
  return f(x);
}
