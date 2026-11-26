// expect: rejected cerastes-array_address_null_test @ L26
// expect: frontend-warning boolean_controlling_expr_is_constant @ L26
// expect: rejected cerastes-array_address_null_test @ L36
// expect: frontend-warning boolean_controlling_expr_is_constant @ L36
// expect: rejected cerastes-array_address_null_test @ L45
// expect: frontend-warning boolean_controlling_expr_is_constant @ L45
// mode: c23
// c-only: in C++ the front end folds an address into a constant and the form is rejected as unsupported_expression ("taking an address ... in an expression the compiler folded to a constant"), a tag that array_address_reject_static_folded.c pins for the other positions; item 5's fix is C's `if` (record 0064 item 5)
// twin: tests/memory/array_address_scalar_ok.c
// Pins: 0064 item 5 (audit round R, should-fix 2; Commit 2): in C a constant index into an array of static storage is folded by the front end into a ck_address constant whose kept expr is the written &E[i], and the predicate reads through it. A truth test of such an address as the condition of an `if` is rejected like the locals' (array_address_scalar_reject.c; GCC -Waddress at -Wall, clang silent): `if (&ga[2])` on a global array, `if (&sa[1])` on a `static` local array, `if (&gs.a[1])` on a global structure's array member. The audit's probes g01, g06 and g07 (build/m4.5e1e2b/audit-R/q322b/): verified at the Commit 1 build (EDG's warnings only), and edg-backend's round R build moves every truth test whose OPERAND is the folded address to the same tag (a loop condition, an operand of `&&`, `||` and `?:`: array_address_reject_static_operands.c), while a test the front end folds whole (`!&ga[1]`, `&ga[1] != NULL`) keeps unsupported_expression (array_address_reject_static_folded.c). Each `if` has no else and nothing after it that the constant makes unreachable, so EDG prints only warning 236 (the constant controlling expression) on its line
// Pre-registered from record 0064 item 5 (audit round R, should-fix 2), before any run: array_address_null_test at the `if`, one row per function; the frontend-warning rows are registered from the audit's note (EDG 236-D on each) and the tag boolean_controlling_expr_is_constant, a question 160 candidate for the warning rows alone (the audit's forms also drew 111-D on the statement after the `if`, which these forms avoid by construction)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int x;
};

int ga[4];
struct S gs;

int global_array(void)
{
  int r = 0;
  if (&ga[2]) {  // <- here
    r = 1;
  }
  return r;
}

int static_local(void)
{
  static int sa[4];
  int r = 0;
  if (&sa[1]) {  // <- here
    r = 1;
  }
  return r;
}

int global_member(void)
{
  int r = 0;
  if (&gs.a[1]) {  // <- here
    r = 1;
  }
  return r;
}
