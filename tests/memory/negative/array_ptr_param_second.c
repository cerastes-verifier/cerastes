// expect: error out-of-bounds @ L13
// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/array_ptr_param_first.c
// Pins: 0059 items 4 and 10 (S43 N1): p[1] with only owns(*p): two real cases at one line, the bound and the permission of the second slot
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code
#include <cerastes.h>

int first(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == *p)
{
  return p[1];
}
