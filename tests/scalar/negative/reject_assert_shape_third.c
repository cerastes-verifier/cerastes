// expect: rejected cerastes-call_without_contract @ L13
// expect: rejected cerastes-unsupported_type @ L13
// mode: c23, c++20
// twin: tests/scalar/assert_holds.c
// Pins: 0015 item 6: the shape of assert's expansion is matched exactly; here the third operand is (void)1, not (void)0
// Observed (build of 2026-10-21): the near-miss falls to the general rules, two diagnostics at the same line.
#include <assert.h>
#include <cerastes.h>

int shape(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  (__builtin_expect(!(x >= 0), 0) ? __assert_rtn(__func__, "f.c", 1, "x >= 0") : (void)1);
  return x;
}
