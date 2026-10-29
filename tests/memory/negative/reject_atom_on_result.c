// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/live_result.c
// Pins: 0037 item 15: an atom on *result is rejected until owning results (re-expected by M4.5d2 or b1c)
#include <cerastes.h>
#include <stddef.h>

int *same(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_live(p) && cerastes_owns(*r))
{
  return p;
}
