// expect: error out-of-bounds @ L14
// expect: error permission @ L14
// mode: c23
// c-only: `int a[const 3]` is C only (a qualifier inside the brackets)
// twin: tests/memory/array_param_accepted_const_bound.c
// Pins: 0060 item 15 (S43 N1): owns(*a) holds one element: a[1] fails its bound and its permission at the read
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int first(int a[const 3])
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, cerastes_owns(*a))
{
  return a[1];
}
