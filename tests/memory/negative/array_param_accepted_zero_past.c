// expect: error out-of-bounds @ L13
// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/array_param_accepted_zero.c
// Pins: 0060 item 15 (S43 N1): owns(*a) holds one element: a[1] fails its bound and its permission at the read
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int first(int a[0])
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, cerastes_owns(*a))
{
  return a[1];
}
