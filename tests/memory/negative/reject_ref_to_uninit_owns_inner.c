// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/ptrptr_owns_inner.c
// Pins: 0045 item 13 (0042 item 6): ref_to_uninit on pp with owns(**pp) in the requires: the contract reads *pp, which the marker says may hold no value on entry
#include <cerastes.h>

void f(int **pp CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))  // <- here
{
  (void)pp;
}
