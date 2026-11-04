// expect: rejected cerastes-call_without_contract @ L12
// c-only: __assert_single_arg is declared only in glibc's C23 <assert.h> (C++'s assert has the static_cast shape); platform: linux
// twin: tests/scalar/assert_holds.c
// Pins: 0031 item 5: the shape of glibc's C23 assert expansion is matched exactly; here the second arm is (void)1, not (void)0
// The near miss falls to the general rules: __assert_fail has no contract.
#include <assert.h>
#include <cerastes.h>

int shape(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  (((void) sizeof (__assert_single_arg (x >= 0)), x >= 0) ? (void) (1) : __assert_fail ("x >= 0", "f.c", 1, __func__));
  return x;
}
