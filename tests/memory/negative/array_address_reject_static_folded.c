// expect: rejected cerastes-unsupported_expression @ L16
// expect: frontend-warning boolean_controlling_expr_is_constant @ L16
// expect: rejected cerastes-unsupported_expression @ L21
// mode: c23
// c-only: EDG prints warning 236 (a constant controlling expression) on `!&ga[1]` in C only; the C++ twin of this file is array_address_reject_static_folded_cpp.cpp
// twin: tests/memory/array_address_scalar_ok.c
// Pins: 0064 item 5 (audit round R, should-fix 2, the positions that already reject): an address the front end folds into a constant (a constant index into an array of static storage) is rejected as unsupported_expression ("taking an address ... in an expression the compiler folded to a constant") when the front end folds the WHOLE test to a constant (in C; edg-backend's round R report: the tag moves only for a truth test whose operand is the folded address, array_address_reject_static.c and _operands.c): `!&ga[1]` and `&ga[1] != NULL` as returned values, in C. No pin held these positions before this file (the audit's probes g02 and g04, build/m4.5e1e2b/audit-R/q322b/: rejected at the Commit 1 build); Commit 2 leaves each with its tag and line while it moves the C truth-test forms to array_address_null_test (array_address_reject_static.c). The tag is the record's own statement ("leaves every row that already rejects with its tag and line")
// Pre-registered from record 0064 item 5 (audit round R, should-fix 2), before any run: unsupported_expression at each return, both modes. NO frontend-warning row is registered: neither form is a controlling expression, so EDG's 236 is not expected, but the audit's saved notes do not list the warnings of these two probes; a warning row, if the first run prints one in either mode, is a question 160 correction for the warning rows alone
#include <cerastes.h>
#include <stddef.h>

int ga[4];

int negation(void)
{
  return !&ga[1];  // <- here
}

int comparison(void)
{
  return &ga[1] != NULL;  // <- here
}
