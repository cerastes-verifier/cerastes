// expect: rejected cerastes-call_without_contract @ L12
// c-only: __assert_single_arg is declared only in glibc's C23 <assert.h> (C++'s assert has the static_cast shape); platform: linux
// twin: tests/scalar/assert_holds.c
// Pins: 0046 (handoff S5 note, 0031 item 10): the sizeof's operand must be a call of __assert_single_arg; a hand-written `sizeof (x++)` is not the macro's (Until the matcher required the call, this hand-written expansion matched and verified; question 119: the sizeof or the arm that is not the macro's is also a side effect that is never evaluated, reported at the same line)
// expect: rejected cerastes-side_effect_not_evaluated @ L12
#include <assert.h>
#include <cerastes.h>

int shape(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  (((void) sizeof (x++), x >= 0) ? (void) (0) : __assert_fail ("x >= 0", "f.c", 1, __func__));
  return x;
}
