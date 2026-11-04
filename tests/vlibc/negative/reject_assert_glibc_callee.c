// expect: rejected cerastes-call_without_contract @ L14
// c-only: __assert_single_arg is declared only in glibc's C23 <assert.h> (C++'s assert has the static_cast shape); platform: linux
// twin: tests/scalar/assert_holds.c
// Pins: 0031 item 5: the failing arm must call glibc's __assert_fail; a user's reporting function in its place makes it no assertion
// The near miss falls to the general rules: the user's function has no contract.
#include <assert.h>
#include <cerastes.h>

void report_failure(const char *what);

int shape(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  (((void) sizeof (__assert_single_arg (x >= 0)), x >= 0) ? (void) (0) : report_failure ("x >= 0"));
  return x;
}
