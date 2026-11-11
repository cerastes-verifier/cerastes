// expect: error out-of-bounds @ L13
// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/array_ptr_callee_write.c
// Pins: 0059 items 4 and 10 (S43 N1): the callee owns only *p, so p[1] fails twice at one line: its bound may be outside the object, and in a larger object nothing gives this function a[1]
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code
#include <cerastes.h>

void set1(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p[1] = 9;
}
