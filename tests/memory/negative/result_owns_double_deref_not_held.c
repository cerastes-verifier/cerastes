// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/result_owns_double_deref.c
// Pins: 0052 item 2 (the framing in the former pin): the callee holds *pp but not **pp, and returns owns(*r) && owns(**r): the first is given, the second is not held, so permission at the return
// Pre-registered from record 0052 item 2: permission at the return statement
#include <cerastes.h>

int **hand(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(r, cerastes_owns(*r) && cerastes_owns(**r) && r == pp)
{
  return pp;  // <- here
}
