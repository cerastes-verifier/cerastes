// expect: rejected cerastes-call_without_contract @ L14
// expect: rejected cerastes-unsupported_type @ L14
// mode: c23, c++20
// twin: tests/scalar/assert_holds.c
// Pins: 0015 item 6: the second operand must call __assert_rtn; abort() has the same effect but another shape
// Observed (build of 2026-10-21): the near-miss falls to the general rules, two diagnostics at the same line.
#include <assert.h>
#include <stdlib.h>
#include <cerastes.h>

int shape(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  (__builtin_expect(!(x >= 0), 0) ? abort() : (void)0);
  return x;
}
