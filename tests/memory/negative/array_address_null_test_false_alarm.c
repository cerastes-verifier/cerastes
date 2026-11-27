// expect: rejected cerastes-array_address_null_test @ L18
// expect: rejected cerastes-array_address_null_test @ L25
// expect: rejected cerastes-array_address_null_test @ L31
// mode: c23
// c-only: the audit measured these three forms in C only (q322 lines 20 and 52-54, q339b line 15); in C++ the rule rejects the same shapes, and the third form, `== (int *)0`, is the one only g++ flags there
// twin: tests/memory/array_address_scalar_ok.c
// Pins: 0064 item 5 (audit round R, note 4; question 342; a false alarm pinned, the programs are correct): a null test of an address formed by an index that NO compiler flags is still rejected, because item 5's predicate reads the expression, not the warning: `assert(&a[1] != NULL)` and a macro `ISNULL(&a[1])` (GCC and clang are silent on a test written through a macro in both modes: the front end does not record that a test came from a macro), and, in C, `&a[1] == (int *)0` (only g++ flags it, in C++). The base accepted all three for an array of scalars; Commit 1 rejects them. Fail closed, listed in the record as over-strict beyond `&p[0]`; M8's warning sweep narrows the rule (the precise set needs the front end to say that a test came from a macro). When the rule is narrowed these rows move to verified and this file is retired with a positive of the same programs
// Pre-registered from record 0064 item 5 (audit round R, note 4; question 342), before any run: array_address_null_test at each expression, the audit's z01, z02 and z03 at the Commit 1 build (rejected there too)
#include <assert.h>
#include <cerastes.h>
#include <stddef.h>

#define ISNULL(e) ((e) == NULL)

int assert_form(void)
{
  int a[4] = {1, 2, 3, 4};
  assert(&a[1] != NULL);  // <- here
  return a[1];
}

int macro_form(void)
{
  int a[4] = {1, 2, 3, 4};
  return ISNULL(&a[1]);  // <- here
}

int typed_null_form(void)
{
  int a[4] = {1, 2, 3, 4};
  return &a[1] == (int *)0;  // <- here
}
