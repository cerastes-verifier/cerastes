// expect: rejected cerastes-call_without_contract @ L13
// (the `rejected cerastes-unsupported_type` line that was here is gone, decision 0025 item 3; this comment keeps the line numbers)
// mode: c23, c++20
// twin: tests/scalar/assert_holds.c
// Pins: 0015 item 6: the shape of assert's expansion is matched exactly; here the third operand is (void)1, not (void)0
// Observed (build of 2026-10-26): the near-miss falls to the general rules, one diagnostic (call_without_contract); the void ?: arm (void)0 is an empty statement since 0025 item 3, so the unsupported_type line it had until M4.4b2 is gone
#include <assert.h>
#include <cerastes.h>

int shape(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  (__builtin_expect(!(x >= 0), 0) ? __assert_rtn(__func__, "f.c", 1, "x >= 0") : (void)1);
  return x;
}
