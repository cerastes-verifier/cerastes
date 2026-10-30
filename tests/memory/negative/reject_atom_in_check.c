// expect: rejected cerastes-call_in_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/loop_owns_fill.c
// Pins: 0038 item 10: a permission atom in CERASTES_CHECK is rejected (never lifted)
#include <cerastes.h>

void check_atom(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  CERASTES_CHECK(cerastes_owns(*p));
}
