// expect: rejected cerastes-array_address_null_test @ L20
// expect: frontend-warning boolean_controlling_expr_is_constant @ L20
// expect: rejected cerastes-array_address_null_test @ L29
// expect: frontend-warning boolean_controlling_expr_is_constant @ L29
// mode: c23
// c-only: in C++ the front end folds an address into a constant and every such form is rejected as unsupported_expression; the wider rule is C's
// twin: tests/memory/array_address_scalar_ok.c
// Pins: 0064 item 5 (audit round R, should-fix 2; edg-backend's round R build, report 6): the rule is wider than the `if` form. In C every truth test whose OPERAND is the folded address of a constant index into an array of static storage is array_address_null_test: the condition of a loop (`while (&ga[2])`, and `for`, `do` alike) and an operand of `&&` (`x && &ga[2]`, which verified before the fix), `||` and `?:`. A test that the front end folds WHOLE to a constant (`!&ga[1]`, `&ga[1] != NULL`, `(bool)&ga[2]`) keeps unsupported_expression (array_address_reject_static_folded.c) and all of C++ is unchanged. One row each for a loop condition and an `&&` operand; the `if`, the `static` local and the member forms are array_address_reject_static.c
// Pre-registered from record 0064 item 5 and edg-backend's round R report, before any run: array_address_null_test at the loop condition and at the `&&` operand. CORRECTED once under question 160 (found by the joined proof j2a before the first harness run, edg-backend report 7): the loop needs a measure (a bare `while (&ga[2])` draws missing_decreases at the loop, so the loop now carries an invariant and a decreases, whose lines moved the rows from L17/L26 to L20/L29), and EDG's warning 236 (boolean_controlling_expr_is_constant) prints at the `&&` operand too, so both rows carry it
#include <cerastes.h>
#include <stddef.h>

int ga[4];

int loop_condition(void)
{
  int r = 0;
  CERASTES_INVARIANT(0 <= r && r <= 1)
  CERASTES_DECREASES(1 - r)
  while (&ga[2]) {  // <- here
    r = 1;
    break;
  }
  return r;
}

int and_operand(int x)
{
  return x && &ga[2];  // <- here
}
