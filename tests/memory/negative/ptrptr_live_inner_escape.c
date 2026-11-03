// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/ptrptr_live_inner.c
// Pins: 0045 item 6 (the old ptrptr_callee_escape.c shape with ensures(live(*pp))): the stored &local dies at the scope exit, so the return does not give back live(*pp): permission at the closing brace
#include <cerastes.h>
#include <stddef.h>

void escape(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_live(*pp))
{
  int local = 1;
  *pp = &local;
}  // <- here
