// expect: rejected cerastes-array_address_null_test @ L22
// expect: rejected cerastes-array_address_null_test @ L31
// expect: rejected cerastes-array_address_null_test @ L38
// expect: rejected cerastes-array_address_null_test @ L45
// expect: rejected cerastes-array_address_null_test @ L52
// expect: rejected cerastes-array_address_null_test @ L59
// mode: c23, c++20
// twin: tests/memory/array_address_scalar_ok.c
// Pins: 0064 item 5 (question 322, the owner): a truth test or a null comparison of an address formed by an index is rejected like the compilers (GCC -Waddress at -Wall on each; clang is silent on all of them) whatever the element: if (&a[1]) on a local array of int, &a[1] == NULL, &p[1] != NULL on an int *p, !&s.a[i] on a member array, &p->a[i] == NULL through a pointer, and &p[0] != NULL on a pointer, which GCC is silent on and the rule rejects with the other indexes (question 335, over-strict by that one cell, listed). Compiler cells: build/m4.5e1e2b/prev/s0/addenda/matrix.json (w_local_scalar, w_ptr_int, w_member_array_local, w_member_array_ptr, w_ctl_addr_p)
// Pre-registered from record 0064 item 5 (Part R), before any run: rejected, one row per function at the expression, array_address_null_test as array_struct_reject_expressions.c has it for arrays of structures
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int local_truth(void)
{
  int a[4] = {1, 2, 3, 4};
  if (&a[1]) {  // <- here
    return a[0];
  }
  return 0;
}

int local_null(void)
{
  int a[4] = {1, 2, 3, 4};
  return (&a[1] == NULL) ? 0 : a[0];  // <- here
}

int pointer_null(int *p)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(r, cerastes_owns_range(p, 2))
{
  return (&p[1] != NULL) ? p[0] : 0;  // <- here
}

int member_negation(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
{
  struct S s = {{1, 2, 3, 4}, 5};
  return !&s.a[i] ? 0 : s.n;  // <- here
}

int member_through_pointer(struct S *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 4)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return (&p->a[i] == NULL) ? 0 : p->n;  // <- here
}

int pointer_first(int *p)
    CERASTES_REQUIRES(cerastes_owns_range(p, 1))
    CERASTES_ENSURES(r, cerastes_owns_range(p, 1))
{
  return (&p[0] != NULL) ? p[0] : 0;  // <- here
}
