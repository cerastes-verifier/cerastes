// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/memory/live_result.c
// Pins: 0037 items 5 and 15: *result as a value is rejected until owning results (re-expected by b1c)
#include <cerastes.h>
#include <stddef.h>

int *same(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_live(p) && *r == 1)
{
  return p;
}
