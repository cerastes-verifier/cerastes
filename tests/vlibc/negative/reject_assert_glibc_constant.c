// expect: rejected cerastes-call_without_contract @ L13
// c-only: __assert_single_arg is declared only in glibc's C23 <assert.h> (C++'s assert has the static_cast shape); platform: linux
// twin: tests/scalar/assert_holds.c
// Pins: 0031 item 5 (audit S5): the left operand of the comma must be the `sizeof` of the macro; any other folded constant there is code the matcher would not encode
// `(void)(INT_MAX + 1)` there would have verified with its overflow unencoded.
#include <assert.h>
#include <limits.h>
#include <cerastes.h>

int shape(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  (((void) (INT_MAX + 1), x >= 0) ? (void) (0) : __assert_fail ("x >= 0", "f.c", 1, __func__));
  return x;
}
