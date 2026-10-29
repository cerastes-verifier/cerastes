// expect: rejected cerastes-spec_call_in_code @ L12
// mode: c23, c++20
// twin: tests/memory/loop_owns_fill.c
// Pins: 0038 item 10: a permission atom in assert is rejected (never lifted)
#include <cerastes.h>
#include <assert.h>

void assert_atom(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  assert(cerastes_owns(*p));
}
