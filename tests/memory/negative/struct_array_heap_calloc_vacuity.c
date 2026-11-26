// expect: error check @ L24
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_malloc.c
// Pins: 0064 item 15 (question 351, item 3: the typed view of calloc must not be vacuous): a structure with an `int a[4]` is allocated with calloc and USED (its last element is read and compared with the view's zero), and then `CERASTES_CHECK(0 == 1)` after it is reported at the check: the view's clauses are satisfiable together, so the state after the call is not impossible (the form of negative/forall_trusted_decl_check_false.c)
// Pre-registered from record 0064 item 15 and question 351, before any run: error check at the check (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int zeroed_then_false(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  int v = p->a[3];
  CERASTES_CHECK(v == 0);
  CERASTES_CHECK(0 == 1);  // <- here
  free(p);
  return v;
}
